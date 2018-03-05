#include <stdio.h>
#include "VROVector3f.h"
#include "VROLog.h"
#include "VROViewScene.h"

int main(int argc, char ** argv) {
#ifdef WASM_PLATFORM
    VROVector3f v(1, 1, 1);
    v = v.scale(2);
	pinfo("Hello, ESM defined, %f", v.x);
    
    VROViewScene *view = new VROViewScene();
#else
    printf("Hello ESM not defined");
#endif
}
