#include "editor.hpp"

namespace engine{
    namespace editor{
        void initialize(Window& window){
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();

            ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow, true);
            ImGui_ImplOpenGL3_Init("#version 430");

            //baseStyle = ImGui::GetStyle();
        }

        void update(Scene& scene){
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            draggedEntity = entt::null;
            if(const ImGuiPayload* payload = ImGui::GetDragDropPayload()){
                if(payload->IsDataType("ENTITY_DND")){
                    draggedEntity = *(const entt::entity*)payload->Data;
                }
            }

            // Registry hierarchy
            {
                ImGui::Begin("Registry Hierarchy");

                if(selectedEntity != entt::null){
                    if(ImGui::Button("Reset Selection")){
                        selectedEntity = entt::null;
                    }

                    if(ImGui::Button("Unparent Selected")){
                        game_object_system::setParent(scene, selectedEntity, entt::null);
                    }
                }

                renderEntity(scene, scene.firstRoot, true, false);

                ImGui::End();
            }

            // Entity inspector
            {
                ImGui::Begin("Entity Inspector");

                if(selectedEntity == entt::null){
                    ImGui::Text("No entity selected.");
                }else{
                    // TODO load components.
                    ImGui::Text("Placeholder");

                    //for(auto metaType : component_types){
                    //}

                    ImGui::Separator();

                    if(ImGui::Button("Add Component")){
                        // TODO
                    }
                }

                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        void renderEntity(Scene& scene, entt::entity entity, const bool& active, const bool& insideDragged){
            while(entity != entt::null){
                GameObject& gameObject = scene.registry.get<GameObject>(entity);
                bool globalActive = active && gameObject.active;
                bool subtreeDragged = insideDragged || entity == draggedEntity;

                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

                if(entity == selectedEntity){
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(gameObject.firstChild == entt::null){
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }

                vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

                if(!globalActive){
                    color.a = 0.5f;
                }

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, color.a));

                bool opened = ImGui::TreeNodeEx(
                    (void*)(uint64_t)entity,
                    flags,
                    "%s",
                    gameObject.name.c_str()
                );

                ImGui::PopStyleColor();

                if(ImGui::IsItemClicked()){
                    selectedEntity = entity;
                }

                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)){
                    ImGui::SetDragDropPayload("ENTITY_DND", &entity, sizeof(entt::entity));
                    ImGui::Text("%s", gameObject.name.c_str());
                    ImGui::EndDragDropSource();
                }

                if(!subtreeDragged && ImGui::BeginDragDropTarget()){
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_DND")){
                        entt::entity droppedEntity = *(const entt::entity*)payload->Data;
                        game_object_system::setParent(scene, droppedEntity, entity);
                    }
                    ImGui::EndDragDropTarget();
                }

                if(opened){
                    renderEntity(
                        scene, 
                        gameObject.firstChild, 
                        globalActive,
                        subtreeDragged
                    );

                    ImGui::TreePop();
                }

                entity = gameObject.nextSibling;
            }
        }

        void shutdown(){
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }
}