
namespace Core
{
class App
{
public:
	/// constructor
	App();
	/// destructor
	~App();

	/// open application
	virtual bool Open();
	/// perform per-frame loop
	virtual void Run();
	/// stop application
	virtual void Close();
	/// kill application
	virtual void Exit();

private:
	bool isOpen;
};
} // namespace App