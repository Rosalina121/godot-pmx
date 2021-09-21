/*************************************************************************/
/*  editor_scene_importer_mmd_pmx.cpp                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "editor_scene_importer_mmd_pmx.h"

#include "thirdparty/ksy/mmd_pmx.h"

#include "core/core_bind.h"
#include "editor/import/scene_importer_mesh_node_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/resources/animation.h"
#include "scene/resources/surface_tool.h"

#include <cstdint>
#include <fstream>
#include <string>

uint32_t EditorSceneImporterMMDPMX::get_import_flags() const {
	return ImportFlags::IMPORT_SCENE;
}

void EditorSceneImporterMMDPMX::get_extensions(List<String> *r_extensions) const {
	r_extensions->push_back("pmx");
}

Node *EditorSceneImporterMMDPMX::import_scene(const String &p_path,
		uint32_t p_flags, int p_bake_fps,
		List<String> *r_missing_deps,
		Error *r_err) {
	Ref<PackedSceneMMDPMX> importer;
	importer.instantiate();
	return importer->import_scene(p_path, p_flags, p_bake_fps, r_missing_deps, r_err, Ref<PMXMMDState>());
}

Ref<Animation> EditorSceneImporterMMDPMX::import_animation(const String &p_path,
		uint32_t p_flags,
		int p_bake_fps) {
	return Ref<Animation>();
}

bool PackedSceneMMDPMX::is_valid_index(mmd_pmx_t::sized_index_t *index) const {
	int64_t bone_index = index->value();
	switch (index->size()) {
		case 1: return bone_index != UINT8_MAX;
		case 2: return bone_index != UINT16_MAX;
		// Have to do SOMETHING even if it's not 4
		default: return bone_index != UINT32_MAX;
	}
}

void PackedSceneMMDPMX::add_vertex(Ref<SurfaceTool> surface, mmd_pmx_t::vertex_t *vertex) const {
	Vector3 normal = Vector3(vertex->normal()->x(),
			vertex->normal()->y(),
			vertex->normal()->z());
	surface->set_normal(normal);
	normal.z = -normal.z;
	Vector2 uv = Vector2(vertex->uv()->x(),
			vertex->uv()->y());
	surface->set_uv(uv);
	Vector3 point = Vector3(vertex->position()->x() * mmd_unit_conversion,
			vertex->position()->y() * mmd_unit_conversion,
			vertex->position()->z() * mmd_unit_conversion);
	point.z = -point.z;
	PackedInt32Array bones;
	bones.push_back(0);
	bones.push_back(0);
	bones.push_back(0);
	bones.push_back(0);
	PackedFloat32Array weights;
	weights.push_back(0.0f);
	weights.push_back(0.0f);
	weights.push_back(0.0f);
	weights.push_back(0.0f);
	if (!vertex->_is_null_skin_weights()) {
		mmd_pmx_t::bone_type_t bone_type = vertex->type();
		switch (bone_type) {
			case mmd_pmx_t::BONE_TYPE_BDEF1: {
				mmd_pmx_t::bdef1_weights_t *pmx_weights = (mmd_pmx_t::bdef1_weights_t *)vertex->skin_weights();
				if (is_valid_index(pmx_weights->bone_index())) {
					bones.write[0] = pmx_weights->bone_index()->value();
					weights.write[0] = 1.0f;
				}
			} break;
			case mmd_pmx_t::BONE_TYPE_BDEF2: {
				mmd_pmx_t::bdef2_weights_t *pmx_weights = (mmd_pmx_t::bdef2_weights_t *)vertex->skin_weights();
				for (uint32_t count = 0; count < 2; count++) {
					if (is_valid_index(pmx_weights->bone_indices()->at(count).get())) {
						bones.write[count] = pmx_weights->bone_indices()->at(count)->value();
						weights.write[count] = pmx_weights->weights()->at(count);
					}
				}
			} break;
			case mmd_pmx_t::BONE_TYPE_BDEF4: {
				mmd_pmx_t::bdef4_weights_t *pmx_weights = (mmd_pmx_t::bdef4_weights_t *)vertex->skin_weights();
				for (uint32_t count = 0; count < RS::ARRAY_WEIGHTS_SIZE; count++) {
					if (is_valid_index(pmx_weights->bone_indices()->at(count).get())) {
						bones.write[count] = pmx_weights->bone_indices()->at(count)->value();
						weights.write[count] = pmx_weights->weights()->at(count);
					}
				}
			} break;
			case mmd_pmx_t::BONE_TYPE_SDEF: {
				// TODO implement 2021-09-10 fire
				mmd_pmx_t::sdef_weights_t *pmx_weights = (mmd_pmx_t::sdef_weights_t *)vertex->skin_weights();
				for (uint32_t count = 0; count < 2; count++) {
					if (is_valid_index(pmx_weights->bone_indices()->at(count).get())) {
						bones.write[count] = pmx_weights->bone_indices()->at(count)->value();
						weights.write[count] = pmx_weights->weights()->at(count);
					}
				}
			} break;
			case mmd_pmx_t::BONE_TYPE_QDEF:
			default: {
				// TODO implement 2021-09-10 fire
				mmd_pmx_t::qdef_weights_t *pmx_weights = (mmd_pmx_t::qdef_weights_t *)vertex->skin_weights();
				for (uint32_t count = 0; count < RS::ARRAY_WEIGHTS_SIZE; count++) {
					if (is_valid_index(pmx_weights->bone_indices()->at(count).get())) {
						bones.write[count] = pmx_weights->bone_indices()->at(count)->value();
						weights.write[count] = pmx_weights->weights()->at(count);
					}
				}
			} break;
		}
		surface->set_bones(bones);
		real_t renorm = weights[0] + weights[1] + weights[2] + weights[3];
		if (renorm != 0.0 && renorm != 1.0) {
			weights.write[0] /= renorm;
			weights.write[1] /= renorm;
			weights.write[2] /= renorm;
			weights.write[3] /= renorm;
		}
		surface->set_weights(weights);
		surface->add_vertex(point);
	}
}

void PackedSceneMMDPMX::_bind_methods() {
	ClassDB::bind_method(D_METHOD("pack_mmd_pmx", "path", "flags", "bake_fps", "state"),
			&PackedSceneMMDPMX::pack_mmd_pmx, DEFVAL(0), DEFVAL(1000.0f), DEFVAL(Ref<PMXMMDState>()));
	ClassDB::bind_method(D_METHOD("import_mmd_pmx_scene", "path", "flags", "bake_fps", "state"),
			&PackedSceneMMDPMX::import_mmd_pmx_scene, DEFVAL(0), DEFVAL(1000.0f), DEFVAL(Ref<PMXMMDState>()));
}
Node *PackedSceneMMDPMX::import_mmd_pmx_scene(const String &p_path, uint32_t p_flags, float p_bake_fps, Ref<PMXMMDState> r_state) {
	Error err = FAILED;
	List<String> deps;
	return import_scene(p_path, p_flags, p_bake_fps, &deps, &err, r_state);
}

Node *PackedSceneMMDPMX::import_scene(const String &p_path, uint32_t p_flags,
		int p_bake_fps,
		List<String> *r_missing_deps,
		Error *r_err,
		Ref<PMXMMDState> r_state) {
	if (r_state == Ref<PMXMMDState>()) {
		r_state.instantiate();
	}
	std::ifstream ifs(
			ProjectSettings::get_singleton()->globalize_path(p_path).utf8().get_data(), std::ifstream::binary);
	kaitai::kstream ks(&ifs);
	mmd_pmx_t pmx = mmd_pmx_t(&ks);
	Node3D *root = memnew(Node3D);
	std::vector<std::unique_ptr<mmd_pmx_t::bone_t> > *bones = pmx.bones();
	Skeleton3D *skeleton = memnew(Skeleton3D);
	uint32_t bone_count = pmx.bone_count();
	for (uint32_t bone_i = 0; bone_i < bone_count; bone_i++) {
		String output_name = convert_string(
				bones->at(bone_i)->name()->value(), pmx.header()->encoding());
		BoneId bone = skeleton->get_bone_count();
		skeleton->add_bone(output_name);
		if (!bones->at(bone_i)->enabled()) {
			skeleton->set_bone_enabled(bone, false);
		}
	}
	for (uint32_t bone_i = 0; bone_i < bone_count; bone_i++) {
		Transform3D xform;
		real_t x = bones->at(bone_i)->position()->x();
		x *= mmd_unit_conversion;
		real_t y = bones->at(bone_i)->position()->y();
		y *= mmd_unit_conversion;
		real_t z = bones->at(bone_i)->position()->z();
		z *= mmd_unit_conversion;
		xform.origin = Vector3(x, y, z);
		int64_t parent_index = -1;
		if (is_valid_index(bones->at(bone_i)->parent_index())) {
			parent_index = bones->at(bone_i)->parent_index()->value();
			real_t parent_x = bones->at(parent_index)->position()->x();
			parent_x *= mmd_unit_conversion;
			real_t parent_y = bones->at(parent_index)->position()->y();
			parent_y *= mmd_unit_conversion;
			real_t parent_z = bones->at(parent_index)->position()->z();
			parent_z *= mmd_unit_conversion;
			xform.origin -= Vector3(parent_x, parent_y, parent_z);
		}
		xform.origin.z = -xform.origin.z;
		skeleton->set_bone_rest(bone_i, xform);
		skeleton->set_bone_parent(bone_i, parent_index);
	}
	root->add_child(skeleton);
	skeleton->set_owner(root);
	std::vector<std::unique_ptr<mmd_pmx_t::material_t> > *materials = pmx.materials();
	Vector<Ref<Texture2D> > texture_cache;
	texture_cache.resize(pmx.texture_count());
	for (uint32_t texture_cache_i = 0; texture_cache_i < pmx.texture_count(); texture_cache_i++) {
		std::string raw_texture_path = pmx.textures()->at(texture_cache_i)->name()->value();
		String texture_path = convert_string(raw_texture_path, pmx.header()->encoding());
		if (!texture_path.is_empty()) {
			texture_path = texture_path.simplify_path();
			Vector<String> path_components = texture_path.split("/");
			texture_path = p_path.get_base_dir();
			core_bind::Directory dir;
			for (const String &elem : path_components) {
				if (dir.open(texture_path) == OK) {
					dir.list_dir_begin();
					String file_name = dir.get_next();
					while (!file_name.is_empty()) {
						if (elem.nocasecmp_to(file_name) == 0) {
							texture_path = texture_path.plus_file(file_name);
							break;
						}
						file_name = dir.get_next();
					}
					if (file_name.is_empty()) {
						print_line(vformat("Couldn't find texture path %s", texture_path));
						break;
					}
				}
			}
			print_verbose(vformat("Found texture %s", texture_path));
			Ref<Texture> base_color_tex = ResourceLoader::load(texture_path);
			texture_cache.write[texture_cache_i] = base_color_tex;
		}
	}

	Vector<Ref<StandardMaterial3D> > material_cache;
	material_cache.resize(pmx.material_count());
	for (uint32_t material_cache_i = 0; material_cache_i < pmx.material_count(); material_cache_i++) {
		Ref<StandardMaterial3D> material;
		material.instantiate();
		String texture_path;
		int64_t texture_index = materials->at(material_cache_i)->texture_index()->value();
		if (is_valid_index(materials->at(material_cache_i)->texture_index()) && texture_index < texture_cache.size()) {
			if (texture_index >= texture_cache.size()) {
				continue;
			}
			if (texture_cache[texture_index].is_null()) {
				continue;
			}
			material->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, texture_cache[texture_index]);
		}
		mmd_pmx_t::color4_t *diffuse = materials->at(material_cache_i)->diffuse();
		material->set_albedo(Color(diffuse->r(), diffuse->g(), diffuse->b(), diffuse->a()));
		String material_name = convert_string(materials->at(material_cache_i)->name()->value(), pmx.header()->encoding());
		material->set_name(material_name);
		material_cache.write[material_cache_i] = material;
	}
	Ref<EditorSceneImporterMesh> mesh;
	mesh.instantiate();

	uint32_t face_start = 0;
	for (uint32_t material_i = 0; material_i < pmx.material_count(); material_i++) {
		Ref<SurfaceTool> surface;
		surface.instantiate();
		surface->begin(Mesh::PRIMITIVE_TRIANGLES);
		const int32_t triangle_vertices = 3;
		std::vector<std::unique_ptr<mmd_pmx_t::vertex_t> > *vertices = pmx.vertices();
		std::vector<std::unique_ptr<mmd_pmx_t::face_t> > *faces = pmx.faces();
		uint32_t face_end = face_start + materials->at(material_i)->face_vertex_count() / 3;
		uint32_t new_count = 0;
		// Add the vertices directly without indices
		for (uint32_t face_i = face_start; face_i < face_end; face_i++) {
			uint32_t index = faces->at(face_i)->indices()->at(0)->value();
			add_vertex(surface, vertices->at(index).get());
			index = faces->at(face_i)->indices()->at(1)->value();
			add_vertex(surface, vertices->at(index).get());
			index = faces->at(face_i)->indices()->at(2)->value();
			add_vertex(surface, vertices->at(index).get());
		}
		Array mesh_array = surface->commit_to_arrays();
		Ref<Material> material = material_cache[material_i];			
		mesh->add_surface(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Dictionary(), material, material->get_name());
		face_start = face_end;
	}
	EditorSceneImporterMeshNode3D *mesh_3d = memnew(EditorSceneImporterMeshNode3D);
	skeleton->add_child(mesh_3d);
	mesh_3d->set_skin(skeleton->register_skin(nullptr)->get_skin());
	mesh_3d->set_mesh(mesh);
	mesh_3d->set_owner(root);
	mesh_3d->set_skeleton_path(NodePath(".."));
	String mesh_name = convert_string(pmx.header()->model_name()->value(), pmx.header()->encoding());
	mesh_3d->set_name(mesh_name);

	std::vector<std::unique_ptr<mmd_pmx_t::rigid_body_t> > *rigid_bodies = pmx.rigid_bodies();
	for (uint32_t rigid_bodies_i = 0; rigid_bodies_i < pmx.rigid_body_count(); rigid_bodies_i++) {
		StaticBody3D *static_body_3d = memnew(StaticBody3D);
		String rigid_name = convert_string(rigid_bodies->at(rigid_bodies_i)->name()->value(), pmx.header()->encoding());
		Transform3D xform;
		Basis basis;
		basis.set_euler(Vector3(
				rigid_bodies->at(rigid_bodies_i)->rotation()->x(),
				rigid_bodies->at(rigid_bodies_i)->rotation()->y(),
				-rigid_bodies->at(rigid_bodies_i)->rotation()->z()));
		xform.basis = basis;
		Vector3 point = Vector3(rigid_bodies->at(rigid_bodies_i)->position()->x() * mmd_unit_conversion,
				rigid_bodies->at(rigid_bodies_i)->position()->y() * mmd_unit_conversion,
				-rigid_bodies->at(rigid_bodies_i)->position()->z() * mmd_unit_conversion);
		xform.origin = point;
		static_body_3d->set_transform(xform);
		static_body_3d->set_name(rigid_name);
		root->add_child(static_body_3d);
		static_body_3d->set_owner(root);
	}
	return root;
}

void PackedSceneMMDPMX::pack_mmd_pmx(String p_path, int32_t p_flags,
		real_t p_bake_fps, Ref<PMXMMDState> r_state) {
	Error err = FAILED;
	List<String> deps;
	Node *root = import_scene(p_path, p_flags, p_bake_fps, &deps, &err, r_state);
	ERR_FAIL_COND(err != OK);
	pack(root);
}

String PackedSceneMMDPMX::convert_string(const std::string &s, uint8_t encoding) const {
	String output;
	if (encoding == 0) {
		Vector<char16_t> buf;
		buf.resize(s.length() / 2);
		memcpy(buf.ptrw(), s.c_str(), s.length());
		output.parse_utf16(buf.ptr(), s.length() / 2);
	} else {
		output.parse_utf8(s.data(), s.length());
	}
	return output;
}
