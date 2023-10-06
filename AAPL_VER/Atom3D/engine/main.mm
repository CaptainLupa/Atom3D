//
//  main.mm
//  Atom3D
//
//  Created by Jens Eckert on 10/4/23.
//

#include "mtl_engine.hpp"

#include <iostream>

int main() {
    
    @autoreleasepool {
        MTLEngine engine;
        
        engine.init();
        engine.run();
        engine.cleanup();
    }
    
    return 0;
}
