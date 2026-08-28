#include "app.hpp"

using namespace engine;

int main(int argc, char** argv){
    bool devMode = false;
    if(argc >= 2){
        if(strcmp(argv[1], "true") == 0){
            devMode = true;
        }
    }
    App app{"Graphics Program", devMode};
    return app.run();
}