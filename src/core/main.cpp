#include <cstring>

#include "app.hpp"

using namespace engine;

int main(int argc, char** argv){
    bool editorOpen = false;
    bool titleDebug = false;
    bool oneFrame = false;
    /*char* vertName;
    char* fragName;*/

    for(int i = 1; i < argc; i++){
        char* arg = argv[i];

        if(strcmp(arg, "editor") == 0){
            editorOpen = true;
        }

        if(strcmp(arg, "tps") == 0){
            titleDebug = true;
        }

        if(strcmp(arg, "static") == 0){
            oneFrame = true;
        }

        /*if(strcmp(arg.substr(0, 5), "vert=") == 0){
            vertName = arg.substr(5, arg.)
        }

        if(strcmp(arg.substr(0, 5), "vert=") == 0){
            fragName = arg.substr(5, arg.)
        }*/
    }

    App app{"Graphics Program", editorOpen, titleDebug, oneFrame};
    return app.run();
}