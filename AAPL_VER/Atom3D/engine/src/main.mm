//
//  main.mm
//  Atom3D
//
//  Created by Jens Eckert on 10/4/23.
//

#include "Core.hpp"

#include <iostream>

int main() {
    
    @autoreleasepool {
        Atom::Core engine;
        
        engine.init();
        engine.run();
        engine.cleanup();
    }
    
    return 0;
}
