#include <STDInclude.hpp>

namespace Assets
{
	void IMaterialTechniqueSet::Mark(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Game::MaterialTechniqueSet* asset = header.materialTechset;

		for (int i = 0; i < ARR_SIZE(Game::MaterialTechniqueSet::techniques); i++)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (!technique) continue;

			for (short j = 0; j < technique->numPasses; j++)
			{
				Game::MaterialPass* pass = &technique->passes[j];

				if (pass->vertexDecl)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl->name);
				}

				if (pass->vertexShader)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader->name);
				}

				if (pass->pixelShader)
				{
					builder->LoadAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader->name);
				}
			}
		}
	}

	void IMaterialTechniqueSet::Save(Game::XAssetHeader header, Components::ZoneBuilder::Zone* builder)
	{
		Assert_Size(Game::MaterialTechniqueSet, 204);

		Utils::Stream* buffer = builder->GetBuffer();
		Game::MaterialTechniqueSet* asset = header.materialTechset;
		Game::MaterialTechniqueSet* dest = buffer->Dest<Game::MaterialTechniqueSet>();
		buffer->Save(asset, sizeof(Game::MaterialTechniqueSet));

		buffer->PushBlock(Game::XFILE_BLOCK_VIRTUAL);

		if (asset->name)
		{
			buffer->SaveString(builder->GetAssetName(this->GetType(), asset->name));
			dest->name = reinterpret_cast<char*>(-1);
		}

		// Save_MaterialTechniquePtrArray
		static_assert(ARR_SIZE(Game::MaterialTechniqueSet::techniques) == 48, "Techniques array invalid!");

		for (int i = 0; i < ARR_SIZE(Game::MaterialTechniqueSet::techniques); i++)
		{
			Game::MaterialTechnique* technique = asset->techniques[i];

			if (technique)
			{
				if (builder->HasPointer(technique))
				{
					dest->techniques[i] = builder->GetPointer(technique);
				}
				else
				{
					// Size-check is obsolete, as the structure is dynamic
					buffer->Align(Utils::Stream::ALIGN_4);
					builder->StorePointer(technique);

					Game::MaterialTechnique* destTechnique = buffer->Dest<Game::MaterialTechnique>();
					buffer->Save(technique, 8);

					// Save_MaterialPassArray
					for (short j = 0; j < technique->numPasses; j++)
					{
						Assert_Size(Game::MaterialPass, 20);

						Game::MaterialPass* destPass = buffer->Dest<Game::MaterialPass>();
						Game::MaterialPass* pass = &technique->passes[j];
						buffer->Save(pass, sizeof(Game::MaterialPass));

						if (pass->vertexDecl)
						{
							destPass->vertexDecl = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_VERTEXDECL, pass->vertexDecl->name).vertexDecl;
						}

						if (pass->vertexShader)
						{
							destPass->vertexShader = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_VERTEXSHADER, pass->vertexShader->name).vertexShader;
						}

						if (pass->pixelShader)
						{
							destPass->pixelShader = builder->RequireAsset(Game::XAssetType::ASSET_TYPE_PIXELSHADER, pass->pixelShader->name).pixelShader;
						}

						if (pass->argumentDef)
						{
							buffer->Align(Utils::Stream::ALIGN_4);
							buffer->SaveArray(pass->argumentDef, pass->argCount1 + pass->argCount2 + pass->argCount3);
							destPass->argumentDef = reinterpret_cast<Game::ShaderArgumentDef*>(-1);
						}
					}

					// We absolutely have to write something here!
					if (technique->name)
					{
						buffer->SaveString(technique->name);
					}
					else
					{
						buffer->SaveString("");
					}

					destTechnique->name = reinterpret_cast<char*>(-1);
					dest->techniques[i] = reinterpret_cast<Game::MaterialTechnique*>(-1);
				}
			}
		}

		buffer->PopBlock();
	}
}
