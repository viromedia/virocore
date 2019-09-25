#include <stdio.h>
#include "VROViewScene.h"
#include "VRORendererTestHarness.h"

int main(int argc, char ** argv) {
#ifdef WASM_PLATFORM
    VROViewScene *view = new VROViewScene(VRORendererTestType::DiffuseIrradiance);
#else
    printf("ESM is not defined! Startup canceled");
#endif
}
