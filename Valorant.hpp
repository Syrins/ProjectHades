#pragma once
namespace VALROM {
	inline std::vector<Enemy> retreiveValidEnemies(uintptr_t actor_array, int actor_count) {
		std::vector<Enemy> temp_enemy_collection{};
		for (int i = 0; i < actor_count; i++) {
			uintptr_t actor = Read<uintptr_t>(actor_array + (i * 0x8));
			if (actor == NULL) {
				continue;
			}

			uintptr_t unique_id = Read<uintptr_t>(actor + offsets::unique_id);
			if (unique_id != 0x11e0101) {
				continue;
			}

			uintptr_t mesh = Read<uintptr_t>(actor + offsets::mesh_component);
			if (!mesh) {
				continue;
			}

			uintptr_t player_state = Read<uintptr_t>(actor + offsets::player_state);
			uintptr_t team_component = Read<uintptr_t>(player_state + offsets::team_component);
			int team_id = Read<int>(team_component + offsets::team_id);
			int bone_count = Read<int>(mesh + offsets::bone_count);

			if (team_id == g_local_team_id) {
				continue;
			}

			uintptr_t damage_handler = Read<uintptr_t>(actor + offsets::damage_handler);
			uintptr_t root_component = Read<uintptr_t>(actor + offsets::root_component);
			uintptr_t bone_array = Read<uintptr_t>(mesh + offsets::bone_array);
			uint32_t bone_array_count = Read<uint32_t>(mesh + offsets::bone_array + 8);
			if (!bone_array_count)
				bone_array = Read<uintptr_t>(mesh + offsets::bone_array_Cached);

			Enemy enemy{
				actor,
				damage_handler,
				player_state,
				root_component,
				mesh,
				bone_array,
				bone_count,
				true
			};
			temp_enemy_collection.push_back(enemy);
		}

		return temp_enemy_collection;
	}
	inline void retreiveData() {
		while (true) {
			if (!IsWindow(valorant_window))
				exit(-1);

			uintptr_t world = decryptWorld(g_base_address);
			auto persistent_level = Read<uintptr_t>(world + offsets::persistent_level);
			auto game_instance = Read<uintptr_t>(world + offsets::game_instance);
			auto local_player_array = Read<uintptr_t>(game_instance + offsets::local_player_array);
			auto local_player = Read<uintptr_t>(local_player_array);
			auto local_player_controller = Read<uintptr_t>(local_player + offsets::local_player_controller);
			auto local_player_pawn = Read<uintptr_t>(local_player_controller + offsets::local_player_pawn);
			auto local_damage_handler = Read<uintptr_t>(local_player_pawn + offsets::damage_handler);
			auto local_player_state = Read<uintptr_t>(local_player_pawn + offsets::player_state);
			auto local_team_component = Read<uintptr_t>(local_player_state + offsets::team_component);
			auto local_team_id = Read<int>(local_team_component + offsets::team_id);
			auto camera_manager = Read<uintptr_t>(local_player_controller + offsets::camera_manager);

			auto actor_array = Read<uintptr_t>(persistent_level + offsets::actor_array);
			auto actor_count = Read<int>(persistent_level + offsets::actor_count);
			g_local_player_controller = local_player_controller;
			g_local_player_pawn = local_player_pawn;
			g_local_damage_handler = local_damage_handler;
			g_camera_manager = camera_manager;
			g_local_team_id = local_team_id;

			enemy_collection = retreiveValidEnemies(actor_array, actor_count);
			Sleep(1500);
		}
	}

	inline Vector3 getBonePosition(Enemy enemy, int index) {

		FTransform Transform = Read<FTransform>(enemy.bone_array_ptr + (0x30 * index));
		FMatrix Matrix = Transform.ToMatrixWithScale() * Read<FTransform>(enemy.mesh_ptr + offsets::component_to_world).ToMatrixWithScale();
		return Vector3(Matrix.WPlane.X, Matrix.WPlane.Y, Matrix.WPlane.Z);
	}
	inline void renderBoneLine(Vector3 first_bone_position, Vector3 second_bone_position, Vector3 position, Vector3 rotation, float fov) {
		Vector2 first_bone_screen_position = worldToScreen(first_bone_position, position, rotation, fov);
		ImVec2 fist_screen_position = ImVec2(first_bone_screen_position.x, first_bone_screen_position.y);
		Vector2 second_bone_screen_position = worldToScreen(second_bone_position, position, rotation, fov);
		ImVec2 second_screen_position = ImVec2(second_bone_screen_position.x, second_bone_screen_position.y);
		ImGui::GetOverlayDrawList()->AddLine(fist_screen_position, second_screen_position, g_Config->g_color_white);
	}
	inline void renderBones(Enemy enemy, Vector3 position, Vector3 rotation, float fov) {
		Vector3 head_position = getBonePosition(enemy, 8);
		Vector3 neck_position;
		Vector3 chest_position = getBonePosition(enemy, 6);
		Vector3 l_upper_arm_position;
		Vector3 l_fore_arm_position;
		Vector3 l_hand_position;
		Vector3 r_upper_arm_position;
		Vector3 r_fore_arm_position;
		Vector3 r_hand_position;
		Vector3 stomach_position = getBonePosition(enemy, 4);
		Vector3 pelvis_position = getBonePosition(enemy, 3);
		Vector3 l_thigh_position;
		Vector3 l_knee_position;
		Vector3 l_foot_position;
		Vector3 r_thigh_position;
		Vector3 r_knee_position;
		Vector3 r_foot_position;
		if (enemy.bone_count == 102) { // MALE & FEMALE
			neck_position = getBonePosition(enemy, 19);

			l_upper_arm_position = getBonePosition(enemy, 21);
			l_fore_arm_position = getBonePosition(enemy, 22);
			l_hand_position = getBonePosition(enemy, 23);

			r_upper_arm_position = getBonePosition(enemy, 47);
			r_fore_arm_position = getBonePosition(enemy, 48);
			r_hand_position = getBonePosition(enemy, 49);

			l_thigh_position = getBonePosition(enemy, 75);
			l_knee_position = getBonePosition(enemy, 76);
			l_foot_position = getBonePosition(enemy, 78);

			r_thigh_position = getBonePosition(enemy, 82);
			r_knee_position = getBonePosition(enemy, 83);
			r_foot_position = getBonePosition(enemy, 85);
		}
		else if (enemy.bone_count == 101) {

			neck_position = getBonePosition(enemy, 21);

			l_upper_arm_position = getBonePosition(enemy, 45);
			l_fore_arm_position = getBonePosition(enemy, 42);
			l_hand_position = getBonePosition(enemy, 44);

			r_upper_arm_position = getBonePosition(enemy, 48);
			r_fore_arm_position = getBonePosition(enemy, 67);
			r_hand_position = getBonePosition(enemy, 69);

			l_thigh_position = getBonePosition(enemy, 75);
			l_knee_position = getBonePosition(enemy, 76);
			l_foot_position = getBonePosition(enemy, 78);

			r_thigh_position = getBonePosition(enemy, 82);
			r_knee_position = getBonePosition(enemy, 84);
			r_foot_position = getBonePosition(enemy, 85);

		}
		else if (enemy.bone_count == 103) { // BOT
			neck_position = getBonePosition(enemy, 9);

			l_upper_arm_position = getBonePosition(enemy, 33);
			l_fore_arm_position = getBonePosition(enemy, 30);
			l_hand_position = getBonePosition(enemy, 32);

			r_upper_arm_position = getBonePosition(enemy, 58);
			r_fore_arm_position = getBonePosition(enemy, 55);
			r_hand_position = getBonePosition(enemy, 57);

			l_thigh_position = getBonePosition(enemy, 63);
			l_knee_position = getBonePosition(enemy, 65);
			l_foot_position = getBonePosition(enemy, 69);

			r_thigh_position = getBonePosition(enemy, 77);
			r_knee_position = getBonePosition(enemy, 79);
			r_foot_position = getBonePosition(enemy, 83);
		}
		else {
			return;
		}

		renderBoneLine(head_position, neck_position, position, rotation, fov);
		renderBoneLine(neck_position, chest_position, position, rotation, fov);
		renderBoneLine(neck_position, l_upper_arm_position, position, rotation, fov);
		renderBoneLine(l_upper_arm_position, l_fore_arm_position, position, rotation, fov);
		renderBoneLine(l_fore_arm_position, l_hand_position, position, rotation, fov);
		renderBoneLine(neck_position, r_upper_arm_position, position, rotation, fov);
		renderBoneLine(r_upper_arm_position, r_fore_arm_position, position, rotation, fov);
		renderBoneLine(r_fore_arm_position, r_hand_position, position, rotation, fov);
		renderBoneLine(chest_position, stomach_position, position, rotation, fov);
		renderBoneLine(stomach_position, pelvis_position, position, rotation, fov);
		renderBoneLine(pelvis_position, l_thigh_position, position, rotation, fov);
		renderBoneLine(l_thigh_position, l_knee_position, position, rotation, fov);
		renderBoneLine(l_knee_position, l_foot_position, position, rotation, fov);
		renderBoneLine(pelvis_position, r_thigh_position, position, rotation, fov);
		renderBoneLine(r_thigh_position, r_knee_position, position, rotation, fov);
		renderBoneLine(r_knee_position, r_foot_position, position, rotation, fov);
	}
	inline void renderBox(Vector2 head_at_screen, float distance_modifier) {
		int head_x = head_at_screen.x;
		int head_y = head_at_screen.y;
		int start_x = head_x - 45 / distance_modifier;
		int start_y = head_y - 25 / distance_modifier;
		int end_x = head_x + 45 / distance_modifier;
		int end_y = head_y + 165 / distance_modifier;
		ImGui::GetOverlayDrawList()->AddRect(ImVec2(start_x, start_y), ImVec2(end_x, end_y), g_Config->g_esp_color);
	}

	inline Vector3 SmoothAim(Vector3 CameraRotation, Vector3 Target, float SmoothFactor)
	{
		Vector3 delta = (Target - CameraRotation).Clamp();

		float smooth = powf(SmoothFactor, 0.4f);
		smooth = min(0.99f, smooth);

		float coeff = (1.0f - smooth) / delta.Length2D() * 0.5f;
		coeff = min(0.99f, coeff);

		return delta * coeff;
	}

	inline static auto string_To_UTF8(const std::string& str) -> std::string
	{
		int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

		wchar_t* pwBuf = new wchar_t[nwLen + 1];
		ZeroMemory(pwBuf, nwLen * 2 + 2);

		::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

		int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

		char* pBuf = new char[nLen + 1];
		ZeroMemory(pBuf, nLen + 1);

		::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

		std::string retStr(pBuf);

		delete[]pwBuf;
		delete[]pBuf;

		pwBuf = NULL;
		pBuf = NULL;

		return retStr;
	}
	inline static auto WStringToUTF8(const wchar_t* lpwcszWString) -> std::string
	{
		char* pElementText;
		int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
		::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
		std::string strReturn(pElementText);
		delete[] pElementText;
		return strReturn;
	}
	inline static auto MBytesToWString(const char* lpcszString) -> std::wstring
	{
		int len = strlen(lpcszString);
		int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
		wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
		std::wstring wString = (wchar_t*)pUnicode;
		delete[] pUnicode;
		return wString;
	}

	static auto DrawNewText(int x, int y, ImColor color, const char* str) -> void
	{
		ImFont a;
		std::string utf_8_1 = std::string(str);
		std::string utf_8_2 = string_To_UTF8(utf_8_1);
		ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color), utf_8_2.c_str());
	}

	inline static auto DrawLine(int x1, int y1, int x2, int y2, ImColor color, int thickness) -> void
	{
		ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), thickness);
	}

	inline static auto DrawFilledRect(int x, int y, int w, int h, ImColor color)-> void
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0);
	}

	inline void DrawFilledRect2(float x, float y, float w, float h, ImColor color)
	{
		ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, NULL, NULL);
	}

	inline void DrawPlayerBar(int x, int y, ImColor back_color, ImColor main_color, const char* pText, ...)
	{
		va_list va_alist;
		char buf[1024] = { 0 };
		va_start(va_alist, pText);
		_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
		va_end(va_alist);
		std::string text = WStringToUTF8(MBytesToWString(buf).c_str());

		const ImVec2 text_dimension = ImGui::CalcTextSize(text.c_str());
		const float text_width = text_dimension.x + 5.f;
		const float mid_width = x - (text_width / 2.f);
		ImColor green_color = { 255, 255, 255,255 };

		DrawFilledRect(mid_width + 2.f, y - 25.f, text_width + 10.f, text_dimension.y + 5.f, back_color);
		DrawLine(mid_width, y - 25.f, mid_width, (y - 25.f) + text_dimension.y + 5.f, green_color, 3.f);
		DrawNewText(mid_width + 6.f, (y - 25.f) + 2.f, main_color, text.c_str());
	}

	inline void Aimbot()
	{
		for (;;)
		{
			if (g_Config->g_Aimbot_enable) {
				while (GetAsyncKeyState(VK_LBUTTON))
				{
					std::vector<Target> possible;

					Vector3 Location = Read<Vector3>(g_camera_manager + offsets::camera_position);
					Vector3 Rocation = Read<Vector3>(g_camera_manager + offsets::camera_rotation);
					float Fov = Read<float>(g_camera_manager + offsets::camera_fov);

					for (int i = 0; i < enemy_collection.size(); ++i)
					{
						Enemy enemy = enemy_collection[i];
						float Health = Read<float>(enemy.damage_handler_ptr + offsets::health);
						if (enemy.actor_ptr == g_local_player_pawn || Health <= 0 || !enemy.mesh_ptr) {
							continue;
						}

						uintptr_t team_component = Read<uintptr_t>(enemy.player_state_ptr + offsets::team_component);
						int team_id = Read<int>(team_component + offsets::team_id);

						if (team_id == g_local_team_id) {
							continue;
						}

						float last_render_time = Read<float>(enemy.mesh_ptr + offsets::last_render_time);
						float last_submit_time = Read<float>(enemy.mesh_ptr + offsets::last_submit_time);
						bool is_visible = last_render_time + 0.06F >= last_submit_time;
						bool dormant = Read<bool>(enemy.actor_ptr + offsets::dormant);
						if (!dormant || !is_visible) {
							continue;
						}

						Vector3 head_position = getBonePosition(enemy, 8); // 8 = head bone
						Vector2 outPos = worldToScreen(head_position, Location, Rocation, Fov);
						Vector3 mPos = { outPos.x - GetSystemMetrics(SM_CXSCREEN) / 2, outPos.y - GetSystemMetrics(SM_CYSCREEN) / 2, 0 };

						float fovDist = mPos.Length();
						if (fovDist <= g_Config->g_AimFov) {
							possible.emplace_back(enemy, head_position, fovDist);
						}
					}

					if (possible.size())
					{
						std::sort(possible.begin(), possible.end(), AimSortCrossHair);

						Target currentTarget = possible[0];

						Vector3 TargetPos = currentTarget.headPos;
						Vector3 TargetRotation = (Location - TargetPos).ToRotator().Clamp();

						Vector3 ConvertRotation = Rocation.Clamp();

						Vector3 ControlRotation = Read<Vector3>(g_local_player_controller + offsets::control_rotation);
						Vector3 DeltaRotation = (ConvertRotation - ControlRotation).Clamp();

						DeltaRotation.y /= 4.0f;
						ConvertRotation = (TargetRotation - (DeltaRotation)).Clamp();

						Vector3 Smoothed = SmoothAim(Rocation, ConvertRotation, g_Config->g_AimSmooth);

						{
							READ_GUARDED_REGION_REQUEST Request = {};
							DWORD cbReturned = 0;
							BOOL status = TRUE;

							ULONGLONG Displacement = (g_local_player_controller + 0x05E4 - 0xC) & 0xFFFFFF;

							if (Displacement < 0x200000)
							{
								uint64_t tmp;

								Request.Displacement = Displacement;
								Request.Buffer = &tmp;
								Request.Size = 8;
								Request.X = Smoothed.x;
								Request.Y = Smoothed.y;

								status = DeviceIoControl(
									mem->hDevice,
									IOCTL_READ_GUARDED_REGION,
									&Request,
									sizeof(Request),
									&Request,
									sizeof(Request),
									&cbReturned,
									NULL);
							}
						}
					}

					Sleep(1);
				}
			}
			if (g_Config->g_Silent_enable) {
				while (GetAsyncKeyState(VK_LBUTTON))
				{
					std::vector<Target> possible;

					Vector3 Location = Read<Vector3>(g_camera_manager + offsets::camera_position);
					Vector3 Rocation = Read<Vector3>(g_camera_manager + offsets::camera_rotation);
					float Fov = Read<float>(g_camera_manager + offsets::camera_fov);

					for (int i = 0; i < enemy_collection.size(); ++i)
					{
						Enemy enemy = enemy_collection[i];
						float Health = Read<float>(enemy.damage_handler_ptr + offsets::health);
						if (enemy.actor_ptr == g_local_player_pawn || Health <= 0 || !enemy.mesh_ptr) {
							continue;
						}

						uintptr_t team_component = Read<uintptr_t>(enemy.player_state_ptr + offsets::team_component);
						int team_id = Read<int>(team_component + offsets::team_id);

						if (team_id == g_local_team_id) {
							continue;
						}

						float last_render_time = Read<float>(enemy.mesh_ptr + offsets::last_render_time);
						float last_submit_time = Read<float>(enemy.mesh_ptr + offsets::last_submit_time);
						bool is_visible = last_render_time + 0.06F >= last_submit_time;
						bool dormant = Read<bool>(enemy.actor_ptr + offsets::dormant);
						if (!dormant || !is_visible) {
							continue;
						}

						Vector3 head_position = getBonePosition(enemy, 8); // 8 = head bone
						Vector2 outPos = worldToScreen(head_position, Location, Rocation, Fov);
						Vector3 mPos = { outPos.x - GetSystemMetrics(SM_CXSCREEN) / 2, outPos.y - GetSystemMetrics(SM_CYSCREEN) / 2, 0 };

						float fovDist = mPos.Length();
						if (fovDist <= g_Config->g_AimFov) {
							possible.emplace_back(enemy, head_position, fovDist);
						}
					}

					if (possible.size())
					{
						std::sort(possible.begin(), possible.end(), AimSortCrossHair);

						Target currentTarget = possible[0];

						Vector3 TargetPos = currentTarget.headPos;
						Vector3 TargetRotation = (Location - TargetPos).ToRotator().Clamp();

						Vector3 ConvertRotation = Rocation.Clamp();

						Vector3 ControlRotation = Read<Vector3>(g_local_player_controller + offsets::control_rotation);
						Vector3 DeltaRotation = (ConvertRotation - ControlRotation).Clamp();

						DeltaRotation.y /= 4.0f;
						ConvertRotation = (TargetRotation - (DeltaRotation)).Clamp();

						Vector3 Smoothed = SmoothAim(Rocation, ConvertRotation, g_Config->g_AimSmooth);

						{
							READ_GUARDED_REGION_REQUEST Request = {};
							DWORD cbReturned = 0;
							BOOL status = TRUE;

							ULONGLONG Displacement = (g_local_player_controller + 0x05E4 - 0xC) & 0xFFFFFF;

							if (Displacement < 0x200000)
							{
								uint64_t tmp;

								Request.Displacement = Displacement;
								Request.Buffer = &tmp;
								Request.Size = 8;
								Request.X = Smoothed.x;
								Request.Y = Smoothed.y;

								status = DeviceIoControl(
									mem->hDevice,
									IOCTL_READ_GUARDED_REGION,
									&Request,
									sizeof(Request),
									&Request,
									sizeof(Request),
									&cbReturned,
									NULL);

								

								static Vector3 OldControlRotation = Smoothed;
								

								if (GetAsyncKeyState(VK_LBUTTON))
								{
									Request.Displacement = Displacement;
									Request.Buffer = &tmp;
									Request.Size = 8;
									Request.X = OldControlRotation.x;
									Request.Y = OldControlRotation.y;
									status = DeviceIoControl(
										mem->hDevice,
										IOCTL_READ_GUARDED_REGION,
										&Request,
										sizeof(Request),
										&Request,
										sizeof(Request),
										&cbReturned,
										NULL);
								}
								else
								{
									OldControlRotation = Smoothed;
								}
							}
						}
					}

					Sleep(1);
				}
			}
		}
	}

	inline void renderEsp() {
		std::vector<Enemy> local_enemy_collection = enemy_collection;
		if (local_enemy_collection.empty()) {
			return;
		}

		Vector3 camera_position = Read<Vector3>(g_camera_manager + offsets::camera_position);
		Vector3 camera_rotation = Read<Vector3>(g_camera_manager + offsets::camera_rotation);
		float camera_fov = Read<float>(g_camera_manager + offsets::camera_fov);

		for (int i = 0; i < local_enemy_collection.size(); i++) {
			Enemy enemy = local_enemy_collection[i];
			float health = Read<float>(enemy.damage_handler_ptr + offsets::health);
			if (enemy.actor_ptr == g_local_player_pawn || health <= 0 || !enemy.mesh_ptr) {
				continue;
			}
			/*const TArray<FTransform> TransformList = Read<TArray<FTransform>>(enemy.mesh_ptr + offsets::bone_array);

			if (TransformList.IsValid() && TransformList.Num() > 90 && TransformList.Num() < 110)
			{
				std::vector<FTransform> vTransformList = TransformList.GetVector();

				FTransform ComponentToWorld = Read<FTransform>(enemy.mesh_ptr + offsets::component_to_world);

				for (size_t x = 0; x < vTransformList.size(); ++x)
				{
					FMatrix Matrix = vTransformList[x].ToMatrixWithScale() * ComponentToWorld.ToMatrixWithScale();
					Vector3 CurrentBone = Vector3{ Matrix.WPlane.X, Matrix.WPlane.Y, Matrix.WPlane.Z };

					{
						Vector2 v2_Location = worldToScreen(CurrentBone, camera_position, camera_rotation, camera_fov);
						ImGui::GetOverlayDrawList()->AddCircle({ v2_Location.x, v2_Location.y }, 5, g_Config->g_esp_color, 0, 3);
					}
				}
			}*/
			Vector3 head_position = getBonePosition(enemy, 8); // 8 = head bone
			Vector3 root_position = getBonePosition(enemy, 0); // 8 = head bone

			if (head_position.z <= root_position.z) {
				continue;
			}

			Vector2 head_at_screen_vec = worldToScreen(head_position, camera_position, camera_rotation, camera_fov);
			ImVec2 head_at_screen = ImVec2(head_at_screen_vec.x, head_at_screen_vec.y);

			Vector2 root_at_screen_vec = worldToScreen(root_position, camera_position, camera_rotation, camera_fov);
			ImVec2 root_at_screen = ImVec2(root_at_screen_vec.x, root_at_screen_vec.y);

			float distance_modifier = camera_position.Distance(head_position) * 0.001F;
			if (g_Config->g_boneesp) {
				renderBones(enemy, camera_position, camera_rotation, camera_fov);
			}
			if (g_Config->g_headesp) {
				ImGui::GetOverlayDrawList()->AddCircle(head_at_screen, 7 / distance_modifier, g_Config->g_esp_color, 0, 3);
			}
			if (g_Config->g_boxesp) {
				renderBox(head_at_screen_vec, distance_modifier);
			}
			if (g_Config->g_Health_bar)
			{
			}
			//float Distance = camera_position.Distance(head_position) / 100.f;
			//char* drawBuff = (char*)malloc(1024);
			//sprintf(drawBuff, ("%0.fm | Player"), Distance);
			//DrawPlayerBar(head_at_screen.x, head_at_screen.y - 15, ImColor(0.125f, 0.125f, 0.125f, 0.35f), ImColor(255, 0, 000, 255), drawBuff);
		}
	}

	inline void TextCentered(std::string text) {
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::Text(text.c_str());
	}

	inline auto TabButton(const char* label, int* index, int val, bool sameline) -> VOID
	{
		if (*index == val)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		}
		if (ImGui::Button(label, ImVec2(120, 40)))
			*index = val;
		if (sameline)
			ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
	}

	inline void runRenderTick() {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();//fff
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

	
		renderEsp();
		ImGui::GetOverlayDrawList()->AddCircle(ImVec2(GetSystemMetrics(SM_CXSCREEN) / 2.0f, GetSystemMetrics(SM_CYSCREEN) / 2.0f), g_Config->g_AimFov, ImColor(255, 255, 255, 255), 30, 0.7);

		static int MenuTab = 0;
		static int VisualTab = 0;


		if (g_Config->g_overlay_visible) {

			static int MenuTab = 0;
			static int VisualTab = 0;

			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4* colors = style->Colors;

			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);





			ImGui::SetNextWindowSize(ImVec2(240, 360));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
			ImGui::Begin("GayLorant", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
				ImGui::Text("");
				ImGui::Checkbox(skCrypt("Enable Aimbot"), &g_Config->g_Aimbot_enable); if (g_Config->g_Aimbot_enable == true) g_Config->g_Silent_enable = false;
				if (g_Config->g_Aimbot_enable or g_Config->g_Silent_enable) {
					ImGui::Checkbox(skCrypt("Fov Circle"), &g_Config->g_DrawFov);
					ImGui::Text("");
					ImGui::SliderFloat(skCrypt("Smooting"), &g_Config->g_AimSmooth, 0.0f, 1.f, skCrypt("Smooth %.2f"));
					ImGui::Text("");
					ImGui::SliderFloat(skCrypt("Fov"), &g_Config->g_AimFov, 0.0f, 300.f, skCrypt("FOV %.2f"));
				
			}


				ImGui::Text("");
				ImGui::Checkbox(skCrypt(u8"Vis Check"), &g_Config->g_esp_dormantcheck);
				ImGui::Checkbox(skCrypt(u8"Head"), &g_Config->g_headesp);
				ImGui::Checkbox(skCrypt(u8"Skeleton"), &g_Config->g_boneesp);
				ImGui::Checkbox(skCrypt(u8"Box"), &g_Config->g_boxesp);

				ImGui::EndChild();
			

			ImGui::End();
		}
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g_window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_window);
	}
}