#include "memory.h"
#include "vector.h"

#include <iostream>
#include <thread>

namespace offset
{
	// client
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDB25DC;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DCDE7C;

	// engine
	constexpr ::std::ptrdiff_t dwClientState = 0x58CFC4;
	constexpr ::std::ptrdiff_t dwClientState_ViewAngles = 0x4D90;
	constexpr ::std::ptrdiff_t dwClientState_GetLocalPlayer = 0x180;

	// entity
	constexpr ::std::ptrdiff_t m_dwBoneMatrix = 0x26A8;
	constexpr ::std::ptrdiff_t m_bDormant = 0xED;
	constexpr ::std::ptrdiff_t m_iTeamNum = 0xF4;
	constexpr ::std::ptrdiff_t m_lifeState = 0x25F;
	constexpr ::std::ptrdiff_t m_vecOrigin = 0x138;
	constexpr ::std::ptrdiff_t m_vecViewOffset = 0x108;
	constexpr ::std::ptrdiff_t m_aimPunchAngle = 0x303C;
	constexpr ::std::ptrdiff_t m_bSpottedByMask = 0x980;
}

// this is the calcualte angle vector and shi
Vector3 CalculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

int main()
{
	const auto memory = Memory{ "csgo.exe" };

	// module addresses fpor client dll and csgo.exe
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");

	// infinite hack loop it would end when you close the program
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		// aimbot keythis would only work if its left shift being held
		if (!GetAsyncKeyState(VK_RBUTTON))
			continue;

		// get local player this woudl get hte player of hte local aplery 
		// this is where there's a large error 
		const auto localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		// this would reutnr 0 fuck 
		std::cout << "0x" << std::hex << localPlayer << std::endl;

		const auto localTeam = memory.Read<std::int32_t>(localPlayer + offset::m_iTeamNum);

		// eye position = origin + viewOffset and this would find the correct offset and shi 
		const auto localEyePosition = memory.Read<Vector3>(localPlayer + offset::m_vecOrigin) +
			memory.Read<Vector3>(localPlayer + offset::m_vecViewOffset);

		const auto clientState = memory.Read<std::uintptr_t>(engine + offset::dwClientState);

		const auto localPlayerId =
			memory.Read<std::int32_t>(clientState + offset::dwClientState_GetLocalPlayer);

		const auto viewAngles = memory.Read<Vector3>(clientState + offset::dwClientState_ViewAngles);
		const auto aimPunch = memory.Read<Vector3>(localPlayer + offset::m_aimPunchAngle) * 2;

		// aimbot fov of the bas and it would comapre iwth the angles and such 
		auto bestFov = 5.f;
		auto bestAngle = Vector3{ };

		for (auto i = 1; i <= 32; ++i)
		{	// this is the differnet between each entity list and shi 
			const auto player = memory.Read<std::uintptr_t>(client + offset::dwEntityList + i * 0x10);

			if (memory.Read<std::int32_t>(player + offset::m_iTeamNum) == localTeam) {
				std::cout << "not youre team type shi" << std::endl;
				continue;
			}

			if (memory.Read<bool>(player + offset::m_bDormant)) {
				std::cout << "player is dormant and such" << std::endl;
				continue;
			}

			if (memory.Read<std::int32_t>(player + offset::m_lifeState)) {
				std::cout << "this is where it doesnt work and such " << std::endl;
				continue;
			}

			if (memory.Read<std::int32_t>(player + offset::m_bSpottedByMask) & (1 << localPlayerId))
			{
				const auto boneMatrix = memory.Read<std::uintptr_t>(player + offset::m_dwBoneMatrix);

				// this would find the player's head position after doint this you can f them up
				const auto playerHeadPosition = Vector3{
					memory.Read<float>(boneMatrix + 0x30 * 8 + 0x0C),
					memory.Read<float>(boneMatrix + 0x30 * 8 + 0x1C),
					memory.Read<float>(boneMatrix + 0x30 * 8 + 0x2C)
				};

				const auto angle = CalculateAngle(
					localEyePosition,
					playerHeadPosition,
					viewAngles + aimPunch
				);

				const auto fov = std::hypot(angle.x, angle.y);

				if (fov < bestFov)
				{
					bestFov = fov;
					bestAngle = angle;
				}
			}
		}

		// this would hceck if its 0 or not otherwise it would write to it 
		if (!bestAngle.IsZero())
			memory.Write<Vector3>(clientState + offset::dwClientState_ViewAngles, viewAngles + bestAngle / 3.f); // smoothing
	}

	return 0;
}