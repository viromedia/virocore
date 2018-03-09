#include <stdio.h>
#include "VROVector3f.h"
#include "VROLog.h"
#include "VROViewScene.h"

int main(int argc, char ** argv) {
#ifdef WASM_PLATFORM
    VROViewScene *view = new VROViewScene();
#else
    printf("ESM is not defined! Startup canceled");
#endif
}
