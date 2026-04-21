#include "SD.h"

SDClass SD;

void File::close() {
    if (_isOpen && _fileName != "") {
        if (_writeBuffer != "") {
            SD.mockSaveFileContent(_fileName.c_str(), _writeBuffer);
        }
    }
    _isOpen = false;
}
