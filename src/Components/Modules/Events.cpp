#include <STDInclude.hpp>

namespace Components
{
	Utils::Signal<Events::ClientCallback> Events::ClientDisconnectSignal;
	Utils::Signal<Events::ClientConnectCallback> Events::ClientConnectSignal;
	Utils::Signal<Events::Callback> Events::SteamDisconnectSignal;
	Utils::Signal<Events::Callback> Events::ShutdownSystemSignal;
	Utils::Signal<Events::Callback> Events::ClientInitSignal;
	Utils::Signal<Events::Callback> Events::ServerInitSignal;
	Utils::Signal<Events::Callback> Events::DvarInitSignal;
	Utils::Signal<Events::Callback> Events::NetworkInitSignal;

	void Events::OnClientDisconnect(const Utils::Slot<ClientCallback>& callback)
	{
		ClientDisconnectSignal.connect(callback);
	}

	void Events::OnClientConnect(const Utils::Slot<ClientConnectCallback>& callback)
	{
		ClientConnectSignal.connect(callback);
	}

	void Events::OnSteamDisconnect(const Utils::Slot<Callback>& callback)
	{
		SteamDisconnectSignal.connect(callback);
	}

	void Events::OnVMShutdown(const Utils::Slot<Callback>& callback)
	{
		ShutdownSystemSignal.connect(callback);
	}

	void Events::OnClientInit(const Utils::Slot<Callback>& callback)
	{
		ClientInitSignal.connect(callback);
	}

	void Events::OnSVInit(const Utils::Slot<Callback>& callback)
	{
		ServerInitSignal.connect(callback);
	}

	void Events::OnDvarInit(const Utils::Slot<Callback>& callback)
	{
		DvarInitSignal.connect(callback);
	}

	void Events::OnNetworkInit(const Utils::Slot<Callback>& callback)
	{
		NetworkInitSignal.connect(callback);
	}

	/*
	 * Should be called when a client drops from the server
	 * but not "between levels" (Quake-III-Arena)
	 */
	void Events::ClientDisconnect_Hk(const int clientNum)
	{
		ClientDisconnectSignal(clientNum);

		Utils::Hook::Call<void(int)>(0x4AA430)(clientNum); // ClientDisconnect
	}

	void Events::SV_UserinfoChanged_Hk(Game::client_s* cl)
	{
		ClientConnectSignal(cl);

		Utils::Hook::Call<void(Game::client_s*)>(0x401950)(cl); // SV_UserinfoChanged
	}

	void Events::SteamDisconnect_Hk()
	{
		SteamDisconnectSignal();

		Utils::Hook::Call<void()>(0x467CC0)(); // LiveSteam_Client_SteamDisconnect
	}

	void Events::Scr_ShutdownSystem_Hk(unsigned char sys)
	{
		ShutdownSystemSignal();

		Utils::Hook::Call<void(unsigned char)>(0x421EE0)(sys); // Scr_ShutdownSystem
	}

	void Events::CL_InitOnceForAllClients_HK()
	{
		ClientInitSignal();
		ClientInitSignal.clear();

		Utils::Hook::Call<void()>(0x404CA0)(); // CL_InitOnceForAllClients
	}

	void Events::SV_Init_Hk()
	{
		ServerInitSignal();
		ServerInitSignal.clear();

		Utils::Hook::Call<void()>(0x474320)(); // SV_InitGameMode
	}

	void Events::Com_InitDvars_Hk()
	{
		DvarInitSignal();
		DvarInitSignal.clear();

		Utils::Hook::Call<void()>(0x60AD10)(); // Com_InitDvars
	}

	void Events::NetworkStart()
	{
		NetworkInitSignal();
		NetworkInitSignal.clear();
	}

	__declspec(naked) void Events::NET_OpenSocks_Hk()
	{
		__asm
		{
			mov eax, 64D900h
			call eax
			jmp NetworkStart
		}
	}

	Events::Events()
	{
		Utils::Hook(0x625235, ClientDisconnect_Hk, HOOK_CALL).install()->quick(); // SV_FreeClient

		Utils::Hook(0x4612BD, SV_UserinfoChanged_Hk, HOOK_CALL).install()->quick(); // SV_DirectConnect

		Utils::Hook(0x403582, SteamDisconnect_Hk, HOOK_CALL).install()->quick(); // CL_Disconnect

		Utils::Hook(0x47548B, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_LoadGame
		Utils::Hook(0x4D06BA, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_ShutdownGame

		Utils::Hook(0x60BE5B, CL_InitOnceForAllClients_HK, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

		Utils::Hook(0x60BB3A, Com_InitDvars_Hk, HOOK_CALL).install()->quick(); // Com_Init_Try_Block_Function

		Utils::Hook(0x4D3665, SV_Init_Hk, HOOK_CALL).install()->quick(); // SV_Init

		Utils::Hook(0x4FD4D4, NET_OpenSocks_Hk, HOOK_JUMP).install()->quick(); // NET_OpenIP
	}
}
