#include "EditorEngine.h"
#include "Core/Application.h"

int main()
{
    CApplication* Application = new CApplication(new CEditorEngine());
    
    Application->Start();

    Application->Run();

    Application->Stop();

    delete Application;

    return 0;
}
