#pragma once

#include <ituGL/application/Application.h>

class FireApplication : public Application
{
public:
	FireApplication();

protected:
	void Initialize() override;
	void Update() override;
	void Render() override;
	void Cleanup() override;
private:
private:
};
