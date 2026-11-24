#pragma once
#include <cstdint> 
#include <deque>
#include <map>
#include <string>
#include <utility> 
#include <vector>
#include "process.h"

class MemoryManager{
    public:
        void init(uint32_t total, uint32_t frameSize);
        
        // Every page of a process into a frame, evict as needed. Returns false if not enough frames for the process
        bool ensureFrames(Process& p, const std::vector<uint32_t>& pages);

        // Byte-level access to the table. Pages must be a resident
        uint16_t read16(const Process& p, uint32_t address) const;
        void write16(Process& p, uint32_t address, uint16_t v);

        // Frees all frames and backing-store pages of a finished/exited process
        void release(Process& p);

        uint32_t frameSize() const {return frameSize_;}
        uint64_t totalBytes() const {return uint64_t(frames_.size()) * frameSize_;}
        uint64_t usedBytes() const;
        uint64_t residentBytes(const Process& p) const;
        uint64_t pagedIn() const {return pagedIn_;}
        uint64_t pagedOut() const {return pagedOut_;}
    
    private:
        struct PhysicalFrame{
            Process* owner = nullptr;
            uint32_t page = 0;
            std::vector<uint8_t> data;
        };

        int freeFrame() const;
        int evict(const Process& p, const std::vector<uint32_t>& pinned);
        void load(Process& p, uint32_t pg, int f);
        void writeBackingFile() const;

        std::vector<PhysicalFrame> frames_;
        std::deque<int> fifo_; // FIFO replacement
        
        std::map<std::pair<int,uint32_t>, std::vector<uint8_t>> store_; // Backing store
        uint32_t frameSize_ = 0;
        uint64_t pagedIn_ = 0;
        uint64_t pagedOut_ = 0;
        
};
