#include "memory.h"
#include <algorithm>
#include <climits>
#include <fstream>
#include <iomanip>
 
// creates (total / frameSize) empty frames
void MemoryManager::init(uint32_t total,uint32_t frameSize){
    frameSize_ = frameSize;
    frames_.assign(total / frameSize, PhysicalFrame{});
    fifo_.clear();
    store_.clear();
    pagedIn_ = pagedOut_ = 0;
    writeBackingFile();
}

// Scans for a frame with no owner and returns its index, or -1 if memory is full
int MemoryManager::freeFrame() const {
    for(size_t i = 0; i < frames_.size(); i++){
        if(!frames_[i].owner) return static_cast<int> (i);
    }
    return -1;
}

// Page-in: copy from the backing store if the page was evicted before, else zero-fill
// (uninitialized memory reads as 0).

void MemoryManager::load(Process& p, uint32_t pg, int f){
    PhysicalFrame& frame = frames_[f];
    auto it = store_.find({p.uniqueId,pg});
    if(it != store_.end()) {
        frame.data = it->second;
        store_.erase(it);
        writeBackingFile();
    }else{
        frame.data.assign(frameSize_,0);
    }
    frame.owner = &p;
    frame.page = pg;
    p.pageTable[pg] = f;
    fifo_.push_back(f);
    ++pagedIn_;
}

// FIFO Victim selection. Skips pages the current instruction still needs so a fault
// can neevr evict a page we are about to use
int MemoryManager::evict(const Process& p, const std::vector<uint32_t>& pinned){
    for(auto it = fifo_.begin(); it != fifo_.end(); ++it){
        PhysicalFrame& f = frames_[*it];
        bool keep = f.owner == &p && std::find(pinned.begin(),pinned.end(),f.page) != pinned.end();
        if (keep) continue;

        int idx = *it;
        fifo_.erase(it);
        store_[{f.owner->uniqueId, f.page}] = f.data;
        f.owner->pageTable[f.page] = -1;
        f.owner = nullptr;
        ++pagedOut_;
        writeBackingFile();
        return idx;
    }
    return -1;
}

// Page fault handler: runs til every page has a valid frame
bool MemoryManager::ensureFrames(Process& p, const std::vector<uint32_t>& pages){
    for(uint32_t pg : pages){
        if(p.pageTable[pg] >= 0) continue; // already a resident
        int f = freeFrame();
        if(f < 0) f = evict(p, pages); // full memory: pick a victim
        if(f < 0) return false;
        load(p,pg,f);
    }
    return true;
}

// --- read and write a uint16 through the page table ---
// === they translate the virtual addresses to a frame and offset
// page = addr / frameSize, offset = addr % frameSize
uint16_t MemoryManager::read16(const Process& p, uint32_t addr) const {
    uint16_t v = 0;
    for(uint32_t b = 0; b < 2; ++b){
        uint32_t a = addr + b;
        const PhysicalFrame& f = frames_[p.pageTable[a/frameSize_]];
        v |= static_cast<uint16_t>(f.data[a % frameSize_]) << (8*b);
    }
    return v;
}

void MemoryManager::write16(Process& p, uint32_t addr, uint16_t v){
    for(uint32_t b = 0; b<2; ++b){
        uint32_t a = addr +b;
        frames_[p.pageTable[a / frameSize_]].data[a % frameSize_] = (v >> (8*b));
    }
}

// called when a process finishes or is killed
// frees all frames, removes them from fifo_, deletes any remaining pages in store_
void MemoryManager::release(Process& p){
    for(size_t pg=0; pg < p.pageTable.size(); ++pg){
        int f = p.pageTable[pg];
        if(f < 0) continue;
        frames_[f].owner = nullptr;
        fifo_.erase(std::remove(fifo_.begin(), fifo_.end(), f), fifo_.end());
        p.pageTable[pg] = -1;
    }
    store_.erase(store_.lower_bound({p.uniqueId, 0}), store_.upper_bound({p.uniqueId, UINT32_MAX}));
    writeBackingFile();
}

uint64_t MemoryManager::usedBytes() const{
    uint64_t n = 0;
    for(const auto& f : frames_) if (f.owner) n += frameSize_;
    return n;
}

// how much of one's process is in RAM
uint64_t MemoryManager::residentBytes(const Process& p) const { 
    uint64_t n = 0;
    for(int f: p.pageTable) if (f >= 0) n += frameSize_;
    return n;
}

// Rewrites csopesy-backing-store.txt with one line per currently evicted page, in hex.
void MemoryManager::writeBackingFile() const {
    std::ofstream out("csopesy-backing-store.txt");
    for (const auto& kv : store_) {
        out << "pid " << kv.first.first << " page " << kv.first.second << ":";
        for (uint8_t b : kv.second)
            out << ' ' << std::hex << std::setw(2) << std::setfill('0') << int(b);
        out << std::dec << '\n';
    }
}
