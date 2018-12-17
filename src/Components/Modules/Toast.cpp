#include "STDInclude.hpp"

namespace Components
{
	std::queue<Toast::UIToast> Toast::Queue;
	std::mutex Toast::Mutex;

	void Toast::Show(const std::string& image, const std::string& title, const std::string& description, int length, Utils::Slot<void()> callback)
	{
		Game::Material* material = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, image.data()).material;
		return Show(material, title, description, length, callback);
	}

	void Toast::Show(Game::Material* material, const std::string& title, const std::string& description, int length, Utils::Slot<void()> callback)
	{
		Toast::Mutex.lock();
		Toast::Queue.push({ material, Utils::String::ToUpper(title), description, length, 0, callback });
		Toast::Mutex.unlock();
	}

	std::string Toast::GetIcon()
	{
		char ourPath[MAX_PATH] = { 0 };
		std::string file(ourPath, GetModuleFileNameA(GetModuleHandle(nullptr), ourPath, sizeof(ourPath)));

		auto pos = file.find_last_of("/\\");
		if (pos != std::string::npos) file = file.substr(0, pos);

		file.append("\\iw4x\\images\\icon.png");
		Utils::String::Replace(file, "/", "\\");
		return file;
	}

	void Toast::Draw(UIToast* toast)
	{
		if (!toast) return;

		int width = Renderer::Width();
		int height = Renderer::Height();
		int slideTime = 100;

		int duration = toast->length;
		int startTime = toast->start;

		int border = 1;
		int cornerSize = 15;
		int bHeight = 74;

		int imgDim = 60;

		float fontSize = 0.9f;
		float descSize = 0.9f;

		Game::Material* white = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, "white").material; if (!white) return;
		Game::Font_s* font = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_FONT, "fonts/objectiveFont").font; if (!font) return;
		Game::Font_s* descfont = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_FONT, "fonts/normalFont").font; if (!descfont) return;
		Game::vec4_t wColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		Game::vec4_t bgColor = { 0.0f, 0.0f, 0.0f, 0.8f };
		Game::vec4_t borderColor = { 1.0f, 1.0f, 1.0f, 0.2f };

		height /= 5;
		height *= 4;

		if (Game::Sys_Milliseconds() < startTime || (startTime + duration) < Game::Sys_Milliseconds()) return;

		// Fadein stuff
		else if (Game::Sys_Milliseconds() - startTime < slideTime)
		{
			int diffH = Renderer::Height() / 5;
			int diff = Game::Sys_Milliseconds() - startTime;
			double scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
			diffH = static_cast<int>(diffH * scale);
			height += diffH;
		}

		// Fadeout stuff
		else if (Game::Sys_Milliseconds() - startTime > (duration - slideTime))
		{
			int diffH = Renderer::Height() / 5;
			int diff = (startTime + duration) - Game::Sys_Milliseconds();
			double scale = 1.0 - ((1.0 * diff) / (1.0 * slideTime));
			diffH = static_cast<int>(diffH * scale);
			height += diffH;
		}

		height += bHeight / 2 - cornerSize;

		// Calculate width data
		int iOffset = (bHeight - imgDim) / 2;
		int iOffsetLeft = iOffset * 2;
		float titleSize = Game::R_TextWidth(toast->title.data(), 0x7FFFFFFF, font) * fontSize;
		float descrSize = Game::R_TextWidth(toast->desc.data(), 0x7FFFFFFF, descfont) * descSize;
		float bWidth = iOffsetLeft * 3 + imgDim + std::max(titleSize, descrSize);

		// Make stuff divisible by 2
		// Otherwise there are overlapping images
		// and I'm too lazy to figure out the actual problem :P
		bWidth = (static_cast<int>(bWidth) + (static_cast<int>(bWidth) % 2)) * 1.0f;
		bHeight += (bHeight % 2);

		// Background
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height - bHeight / 2), bWidth * 1.0f, bHeight * 1.0f, 0, 0, 1.0f, 1.0f, bgColor, white);

		// Border
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2 - border), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0, 0, 1.0f, 1.0f, borderColor, white); // Left
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2 + bWidth), static_cast<float>(height - bHeight / 2 - border), border * 1.0f, bHeight + (border * 2.0f), 0, 0, 1.0f, 1.0f, borderColor, white); // Right
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height - bHeight / 2 - border), bWidth * 1.0f, border * 1.0f, 0, 0, 1.0f, 1.0f, borderColor, white);                      // Top
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2), static_cast<float>(height + bHeight / 2), bWidth * 1.0f, border * 1.0f, 0, 0, 1.0f, 1.0f, borderColor, white);                               // Bottom

		// Image
		Game::Material* image = toast->image;
		if (!Materials::IsValid(image)) image = Game::DB_FindXAssetDefaultHeaderInternal(Game::XAssetType::ASSET_TYPE_MATERIAL).material;
		Game::CL_DrawStretchPicPhysical(static_cast<float>(width / 2 - bWidth / 2 + iOffsetLeft), static_cast<float>(height - bHeight / 2 + iOffset), imgDim * 1.0f, imgDim * 1.0f, 0, 0, 1.0f, 1.0f, wColor, image);

		// Text
		float leftText = width / 2 - bWidth / 2 - cornerSize + iOffsetLeft * 2 + imgDim;
		float rightText = width / 2 + bWidth / 2 - cornerSize - iOffsetLeft;
		Game::R_AddCmdDrawText(toast->title.data(), 0x7FFFFFFF, font, static_cast<float>(leftText + (rightText - leftText) / 2 - titleSize / 2 + cornerSize), static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 7), fontSize, fontSize, 0, wColor, Game::ITEM_TEXTSTYLE_SHADOWED); // Title
		Game::R_AddCmdDrawText(toast->desc.data(), 0x7FFFFFFF, descfont, leftText + (rightText - leftText) / 2 - descrSize / 2 + cornerSize, static_cast<float>(height - bHeight / 2 + cornerSize * 2 + 33), descSize, descSize, 0, wColor, Game::ITEM_TEXTSTYLE_SHADOWED); // Description
	}

	void Toast::Handler()
	{
		if (Toast::Queue.empty()) return;

		std::lock_guard<std::mutex> _(Toast::Mutex);

		Toast::UIToast* toast = &Toast::Queue.front();

		// Set start time
		if (!toast->start)
		{
			toast->start = Game::Sys_Milliseconds();
		}

		if ((toast->start + toast->length) < Game::Sys_Milliseconds())
		{
			if (toast->callback) toast->callback();
			Toast::Queue.pop();
		}
		else
		{
			Toast::Draw(toast);
		}
	}

	Toast::Toast()
	{
		if (Dedicated::IsEnabled() || Monitor::IsEnabled()) return;

		Scheduler::OnReady([]()
		{
			Scheduler::OnFrame(Toast::Handler);
		});

		Command::Add("testtoast", [](Command::Params*)
		{
			Toast::Show("cardicon_prestige10", "Test", "This is a test toast", 3000);
		});
	}

	Toast::~Toast()
	{
		Toast::Queue = std::queue<Toast::UIToast>();
	}
}
