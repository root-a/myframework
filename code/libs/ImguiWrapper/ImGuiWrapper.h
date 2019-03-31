
namespace Display {
	class Window;
};

class ImGuiWrapper
{
public:
	ImGuiWrapper(Display::Window* window);
	~ImGuiWrapper();
	void NewFrame();
	void Render();
private:

};