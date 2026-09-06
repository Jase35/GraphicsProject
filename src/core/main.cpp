#include "app.hpp"

using namespace engine;

int main(int argc, char** argv){
    bool editorOpen = false;
    bool titleDebug = false;
    if(argc >= 3){
        if(strcmp(argv[1], "true") == 0){
            editorOpen = true;
        }
        if(strcmp(argv[2], "true") == 0){
            titleDebug = true;
        }
    }
    App app{"Graphics Program", editorOpen, titleDebug};
    return app.run();
}