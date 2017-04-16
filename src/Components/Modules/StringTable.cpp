#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::string, Game::StringTable*> StringTable::StringTableMap;

	int StringTable::Hash(const char* data)
	{
		int hash = 0;

		while (*data != 0)
		{
			hash = tolower(*data) + (31 * hash);

			++data;
		}

		return hash;
	}

	Game::StringTable* StringTable::LoadObject(std::string filename)
	{
		Utils::Memory::Allocator* allocator = Loader::GetAlloctor();

		filename = Utils::String::ToLower(filename);

		Game::StringTable* table = nullptr;
		FileSystem::File rawTable(filename);

		if (rawTable.exists())
		{
			Utils::CSV parsedTable(rawTable.getBuffer(), false, false);

			table = allocator->allocate<Game::StringTable>();

			if (table)
			{
				table->name = allocator->duplicateString(filename);
				table->columnCount = parsedTable.getColumns();
				table->rowCount = parsedTable.getRows();

				table->values = allocator->allocateArray<Game::StringTableCell>(table->columnCount * table->rowCount);

				if (!table->values)
				{
					return nullptr;
				}

				for (int i = 0; i < table->rowCount; ++i)
				{
					for (int j = 0; j < table->columnCount; ++j)
					{
						std::string value = parsedTable.getElementAt(i, j);

						Game::StringTableCell* cell = &table->values[i * table->columnCount + j];
						cell->hash = StringTable::Hash(value.data());
						cell->string = allocator->duplicateString(value);
						//if (!cell->string) cell->string = ""; // We have to assume it allocated successfully
					}
				}

				StringTable::StringTableMap[filename] = table;
			}
		}
		else
		{
			StringTable::StringTableMap[filename] = nullptr;
		}

		return table;
	}

	StringTable::StringTable()
	{
		AssetHandler::OnFind(Game::XAssetType::ASSET_TYPE_STRINGTABLE, [] (Game::XAssetType, std::string filename)
		{
			Game::XAssetHeader header = { nullptr };

			filename = Utils::String::ToLower(filename);

			if (StringTable::StringTableMap.find(filename) != StringTable::StringTableMap.end())
			{
				header.stringTable = StringTable::StringTableMap[filename];
			}
			else
			{
				header.stringTable = StringTable::LoadObject(filename);
			}

			return header;
		});
	}

	StringTable::~StringTable()
	{
		StringTable::StringTableMap.clear();
	}
}
