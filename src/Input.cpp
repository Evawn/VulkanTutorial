#include "Input.h"

std::vector<Context> Input::contexts;
std::unordered_map<int, KeyState> Input::key_states;
std::vector<std::pair<int, KeyState>> Input::key_changes;
GLFWwindow* Input::window;
InputQuery Input::last_query;

void Input::Init(GLFWwindow* window) {
	Input::window = window;
	key_states = std::unordered_map<int, KeyState>();
	contexts = std::vector<Context>();

	glfwSetKeyCallback(window, KeyCallback);
	Poll();
}

InputQuery Input::Poll() {
	std::vector<int> actions;

	// Record the changes in key states using callback
	key_changes.clear();
	glfwPollEvents();

	// Process PRESSED and RELEASED states
	for (auto& change : key_changes) {
		int key = change.first;
		KeyState state = change.second;

		key_states[key] = state;

		for (auto& context : contexts) {
			if (context.bindings.find(change) != context.bindings.end()) {
				actions.push_back(context.bindings[change]);
				break;
			}
		}
	}

	// Process DOWN and UP states
	for (auto& pair : key_states) {
		auto key = pair.first;
		auto state = pair.second;
		if (state == KeyState::PRESSED) 
			key_states[key] = KeyState::DOWN;
		else if (state == KeyState::RELEASED) 
			key_states[key] = KeyState::UP;

		auto newpair = std::make_pair(key, key_states[key]);
		for (auto& context : contexts) {
			if (context.bindings.find(newpair) != context.bindings.end()) {
				actions.push_back(context.bindings[newpair]);
				break;
			}
		}
	}
	last_query.actions = actions;

	// Process mouse
	double x, y, dx, dy;
	glfwGetCursorPos(window, &x, &y);
	

	if (last_query.center_mouse) {
		/*int win_x, win_y;
		glfwGetWindowPos(window, &win_x, &win_y);*/
		int win_width, win_height;
		glfwGetWindowSize(window, &win_width, &win_height);
		double center_x = win_width / 2;
		double center_y = win_height / 2;

		dx = x - center_x;
		dy = y - center_y;
		x = center_x;
		y = center_y;

		glfwSetCursorPos(window, x, y);
	}
	else {
		dx = x - last_query.x;
		dy = y - last_query.y;
	}

	last_query.x = x;
	last_query.y = y;
	last_query.dx = dx;
	last_query.dy = dy;

	return last_query;
}