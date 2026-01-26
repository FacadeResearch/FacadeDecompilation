#include "SoundSource.h"
#include <cstdio>
#include "Globals.h"

SoundSource::SoundSource() {
    this->m_vtable = 0;
    this->soundCount = 0;

    memset(this->sourcePath, 0, sizeof(this->sourcePath));
    memset(this->baseSoundName, 0, sizeof(this->baseSoundName));
    memset(this->soundList, 0, sizeof(this->soundList));
}

int __cdecl SoundSource::InitSoundSource(char* srcPath)
{
    strncpy(this->sourcePath, srcPath, sizeof(this->sourcePath) - 1);

    FILE* fileStream = fopen_UnixPath(this->sourcePath, "rt");
    if (!fileStream) {
        return dprintf("Could not open file for reading: %s\n", this->sourcePath);
    }

    this->soundCount = -1;

    char lineBuffer[536];
    while (fgets(lineBuffer, 512, fileStream)) {
        if (this->soundCount > 4999) {
            dprintf("Too many sounds in %s", (const char*)fileStream);
            return fclose(fileStream);
        }

        lineBuffer[255] = '\0';

        size_t len = strlen(lineBuffer);
        while (len > 0 && (lineBuffer[len - 1] == 13 || lineBuffer[len - 1] == 10)) {
            lineBuffer[len - 1] = '\0';
            len--;
        }

        if (this->soundCount < 0) {
            strcpy(this->baseSoundName, lineBuffer);
        }
        else {
            strcpy(this->soundList[this->soundCount], lineBuffer);
        }

        this->soundCount++;
    }

    return fclose(fileStream);
}
