#pragma once
 
class FileExplorer
{
public:
    void draw(bool* show);
 
private:
    int selectedFolder = 0;   
    int selectedFile = -1;    
};