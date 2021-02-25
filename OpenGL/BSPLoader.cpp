#include "BSPLoader.h"
#include "stb_image.h"

std::vector<unsigned int> BSPLoader::get_indices()
{
	std::vector<unsigned int> indices;

	// loop all the faces
	for (int i = 0; i < file_faces.size(); ++i)
	{
		auto face = get_face(i);

		// only handle polygon + mesh types at the moment, not patches or billboards.
		if (face.type == 1 || face.type == 3)
		{
			// add to the list of indicies based on the meshvert data
			for (int j = 0; j < face.n_meshverts; ++j)
			{
				// meshvert list should translate directly into triangles.
				int vertIndex = face.meshvert + j;
				int index = face.vertex + file_meshverts[vertIndex].offset;
				indices.push_back(index);
			}
		}
	}

	return indices;
}

void BSPLoader::get_lump_position(int index, int& offset, int& length)
{
	offset = file_directory.direntries[index].offset;
	length = file_directory.direntries[index].length;
}

void BSPLoader::process_textures()
{
	for (int i = 0; i < file_textures.size(); i++)
	{
		texture texture = file_textures[i];
		shader _shader;
		_shader.render = true;
		_shader.solid = true;
		_shader.transparent = false;
		_shader.name = file_textures[i].name;
		if (texture.flags & SURF_NONSOLID) _shader.solid = false;
		if (texture.contents & CONTENTS_PLAYERCLIP) _shader.solid = true;
		if (texture.contents & CONTENTS_TRANSLUCENT) _shader.transparent = true;
		if (texture.contents & CONTENTS_LAVA  || texture.contents & CONTENTS_WATER ||
			texture.contents & CONTENTS_SLIME || texture.contents & CONTENTS_FOG )
				_shader.render = false;
		if (_shader.name == "noshader") _shader.render = false;

		shaders.push_back(_shader);
	}
}

void BSPLoader::process_lightmaps()
{
	for (int i = 0; i < file_lightmaps.size(); ++i)
	{
		const int size = (128 * 128 * 3);
		LightMap map;
		glGenTextures(1, &map.id);
		glBindTexture(GL_TEXTURE_2D, map.id);
		GLenum format = GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, 128, 128, 0, format, GL_UNSIGNED_BYTE, file_lightmaps[i].map);
		glGenerateMipmap(GL_TEXTURE_2D);
		lightmaps.push_back(map);
	}
}

void BSPLoader::load_file()
{
	// open saved file for reading as binary
	std::ifstream fs{ file, std::fstream::in | std::fstream::binary };

	// read directory block
	fs.read( (char*)&file_directory, sizeof(Directory));

	// then read each of the data lumps in "order"
	get_lump_position(0, offset, length);

	file_entities.ents = new char[length];

	fs.seekg(offset);
	fs.read(file_entities.ents, length);

	// 1 to 15 are array based lumps
	read_lump<texture>(1, file_textures, fs);
	read_lump<plane>(2, file_planes, fs);
	read_lump<node>(3, file_nodes, fs);
	read_lump<leaf>(4, file_leafs, fs);
	read_lump<leafface>(5, file_leaffaces, fs);
	read_lump<leafbrush>(6, file_leafbrushes, fs);
	read_lump<model>(7, file_models, fs);
	read_lump<brush>(8, file_brushes, fs);
	read_lump<brushside>(9, file_brushsides, fs);
	read_lump<vertex>(10, file_vertices, fs);
	read_lump<meshvert>(11, file_meshverts, fs);
	read_lump<effect>(12, file_effects, fs);
	read_lump<face>(13, file_faces, fs);
	read_lump<lightmap>(14, file_lightmaps, fs);
	read_lump<lightvol>(15, file_lightvols, fs);

	// 16 is vis data
	get_lump_position(16, offset, length);

	fs.seekg(offset, std::ios_base::beg);
	fs.read((char*)&file_visdata, sizeof(int) * 2);
	int nvecs = file_visdata.n_vecs;
	int sz_vecs = file_visdata.sz_vecs;
	int sz = nvecs * sz_vecs;
	file_visdata.vecs.resize(sz);
	fs.read((char*)&file_visdata.vecs[0], sz);
	fs.close();

	process_textures();
	process_lightmaps();
}

