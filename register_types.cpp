#include "register_types.h"
#include "core/object/class_db.h"
#include "editor/editor_node.h"

#include "editor_scene_importer_mmd_pmx.h"

#ifndef _3D_DISABLED
#ifdef TOOLS_ENABLED
static void _editor_init() {
	Ref<EditorSceneImporterMMDPMX> import_pmx;
	import_pmx.instantiate();
	ResourceImporterScene::get_singleton()->add_importer(import_pmx);
}
#endif
#endif

void register_pmx_types() {
#ifndef _3D_DISABLED
#ifdef TOOLS_ENABLED
	ClassDB::APIType prev_api = ClassDB::get_current_api();
	ClassDB::set_current_api(ClassDB::API_EDITOR);
	GDREGISTER_CLASS(EditorSceneImporterMMDPMX);
	ClassDB::set_current_api(prev_api);
	EditorNode::add_init_callback(_editor_init);
#endif
	GDREGISTER_CLASS(PMXMMDState);
	GDREGISTER_CLASS(PackedSceneMMDPMX);
#endif
}

void unregister_pmx_types() {
}