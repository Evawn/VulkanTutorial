#pragma once
#include <unordered_map>
#include <utility>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

enum class KeyState {
	UP, DOWN, PRESSED, RELEASED
};

class Context {

public:
	std::string name;
	std::map<std::pair<int, KeyState>, int> bindings;
};

struct InputQuery {
	std::vector<int> actions;
	double x, y, dx, dy;
	bool center_mouse, hide_mouse;
};

class Input
{
private:
	static GLFWwindow* window;
	static std::vector<Context> contexts;
	static std::unordered_map<int, KeyState> key_states;
	static std::vector<std::pair<int, KeyState>> key_changes;
	static InputQuery last_query;

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if(action == GLFW_REPEAT) return;
		auto change = (action == GLFW_PRESS ? KeyState::PRESSED : KeyState::RELEASED);
		key_changes.push_back(std::make_pair(key, change));
	}

public:
	static void Init(GLFWwindow* window);

	static void AddContext(Context context) {
		contexts.push_back(context);
	}

	static InputQuery Poll();

	static void HideCursor(bool b) {
		last_query.hide_mouse = b;
	}

	static void CenterCursor(bool b) {
		last_query.center_mouse = b;
		if(b) {
			int w, h;
			glfwGetWindowSize(window, &w, &h);
			glfwSetCursorPos(window, w / 2, h / 2);
		}
	}
};