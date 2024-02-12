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

	// Process mouse
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	double dx = x - last_query.x;
	double dy = y - last_query.y;

	last_query = { actions, x, y, dx , dy };

	return last_query;
}