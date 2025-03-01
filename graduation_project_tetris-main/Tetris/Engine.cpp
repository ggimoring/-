#include "framework.h"
#include "Matrix.h"
#include "Stack.h"
#include "Piece.h"
#include "Engine.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "Windowscodecs.lib")

Engine::Engine() : m_pDirect2dFactory(NULL), m_pRenderTarget(NULL)
{
	// Constructor
	// Initialize your game elements here

    srand(time(NULL));
	
    stack = new Stack();
    stack2 = new Stack();

    activePiece = new Piece();
    activePiece->Activate();
    waitingPiece = new Piece();

    activePiece2 = new Piece();
    activePiece2->Activate();
    waitingPiece2 = new Piece();

    //블록 떨어지는 속도
    autoFallDelay = 0.7;
    autoFallAccumulated = 0;
    keyPressDelay = 0.07;
    keyPressAccumulated = 0;

    autoFallDelay2 = 0.7;
    autoFallAccumulated2 = 0;
    keyPressDelay2 = 0.07;
    keyPressAccumulated2 = 0;

}

Engine::~Engine()
{
    // Destructor

    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);

    // Safe-release your game elements here
    delete stack;
    delete waitingPiece;
    delete activePiece;
    delete stack2;
    delete waitingPiece2;
    delete activePiece2;
}

HRESULT Engine::InitializeD2D(HWND m_hwnd)
{
    // Initializes Direct2D, for drawing
    D2D1_SIZE_U size = D2D1::SizeU(RESOLUTION_X, RESOLUTION_Y);
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
    m_pDirect2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(m_hwnd, size, D2D1_PRESENT_OPTIONS_IMMEDIATELY),
        &m_pRenderTarget
    );

    // Initialize the D2D part of your game elements here
    InitializeTextAndScore();
    stack->InitializeD2D(m_pRenderTarget);
    activePiece->InitializeD2D(m_pRenderTarget);
    waitingPiece->InitializeD2D(m_pRenderTarget);

    stack2->InitializeD2D(m_pRenderTarget);
    activePiece2->InitializeD2D(m_pRenderTarget);
    waitingPiece2->InitializeD2D(m_pRenderTarget);

    return S_OK;
}

void Engine::InitializeTextAndScore()
{
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(m_pDWriteFactory),
        reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
    );

    m_pDWriteFactory->CreateTextFormat(
        L"Verdana",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        20,
        L"", //locale
        &m_pTextFormat
    );

    m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

    m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        &m_pWhiteBrush
    );
}

void Engine::KeyUp(WPARAM wParam)
{
    // If keyup, un-set the keys flags
    // Don't do any logic here, you want to control the actual logic in the Logic method below
    if (wParam == VK_DOWN)
        downPressed2 = false;

    if (wParam == VK_LEFT)
        leftPressed2 = false;

    if (wParam == VK_RIGHT)
        rightPressed2 = false;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = false;

    if (wParam == 83)
        downPressed = false;

    if (wParam == 65)
        leftPressed = false;

    if (wParam == 68)
        rightPressed = false;

    if (wParam == VK_SPACE || wParam == 87)
        spacePressed = false;
}

void Engine::KeyDown(WPARAM wParam)
{
	// If keyup, set the keys flags
	// Don't do any logic here, you want to control the actual logic in the Logic method below
    if (wParam == VK_DOWN)
        downPressed2 = true;

    if (wParam == VK_LEFT)
        leftPressed2 = true;

    if (wParam == VK_RIGHT)
        rightPressed2 = true;

    if (wParam == VK_RETURN || wParam == VK_UP)
        spacePressed2 = true;

    if (wParam == 83)
        downPressed = true;

    if (wParam == 65)
        leftPressed = true;

    if (wParam == 68)
        rightPressed = true;

    if (wParam == VK_SPACE || wParam == 87)
        spacePressed = true;
}

void Engine::MousePosition(int x, int y)
{
    // Campture mouse position when the mouse moves
    // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::MouseButtonUp(bool left, bool right)
{
    // If mouse button is released, set the mouse flags
    // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::MouseButtonDown(bool left, bool right)
{
        // If mouse button is pressed, set the mouse flags
        // Don't do any logic here, you want to control the actual logic in the Logic method below
}

void Engine::Logic(double elapsedTime)
{
    // This is the logic part of the engine. It receives the elapsed time from the app class, in seconds.
    // It uses this value for a smooth and consistent movement, regardless of the CPU or graphics speed

    if (gameOver || gameOver2) // Do nothing if game over
    {
        over = true;

        return;
    }

    // We will need the stack in several places below
    Matrix* stackCells = stack->GetCells();
    Matrix* stackCells2 = stack2->GetCells2();

    // Due to a high FPS, we can't consider the keys at every frame because movement will be very fast
    // So we're using a delay, and if enough time has passed we take a key press into consideration
    keyPressAccumulated += elapsedTime;
    if (keyPressAccumulated > keyPressDelay)
    {
        keyPressAccumulated = 0;
        
        if (leftPressed || rightPressed || spacePressed)
        {
            // Remove any full rows
            int removed = stack->RemoveLines(stackCells);
            if (removed > 0)
            {
                score += pow(2, removed) * 100;
                autoFallDelay = autoFallDelay * 0.98;
            }
        }

        // Move left or right
        if (leftPressed)
            activePiece->GoLeft(stackCells);
        if (rightPressed)
            activePiece->GoRight(stackCells);

        // Rotate
        if (spacePressed)
        {
            activePiece->Rotate(stackCells);
            spacePressed = false;
        }

        // Move down
        // On this one we will just set autoFallAccumulated to be high, because we have the down movemenet logic below
        if (downPressed)
            autoFallAccumulated = autoFallDelay + 1;
    }
    
    keyPressAccumulated2 += elapsedTime;
    if (keyPressAccumulated2 > keyPressDelay2)
    {
        keyPressAccumulated2 = 0;
        
        if (leftPressed2 || rightPressed2 || spacePressed2)
        {
            // Remove any full rows
            int removed = stack->RemoveLines(stackCells2);
            if (removed > 0)
            {
                score += pow(2, removed) * 100;
                autoFallDelay2 = autoFallDelay2 * 0.98;
            }
        }

        // Move left or right
        if (leftPressed2)
            activePiece2->GoLeft2(stackCells2);
        if (rightPressed2)
            activePiece2->GoRight2(stackCells2);

        // Rotate
        if (spacePressed2)
        {
            activePiece2->Rotate2(stackCells2);
            spacePressed2 = false;
        }

        // Move down
        // On this one we will just set autoFallAccumulated to be high, because we have the down movemenet logic below
        if (downPressed2)
            autoFallAccumulated2 = autoFallDelay2 + 1;
    }


    // The piece falls automatically after a delay
    autoFallAccumulated += elapsedTime;
    if (autoFallAccumulated > autoFallDelay) //여기
    {
       autoFallAccumulated = 0;

        // Remove any full rows
        int removed = stack->RemoveLines(stackCells);
        if (removed > 0)
        {
            score += pow(2, removed) * 100;
            autoFallDelay = autoFallDelay * 0.98;
        }

        // Move down the active piece
        bool isConflict = activePiece->Advance(stackCells);
        // If we have a conflict with the stack, it means we were sitting on the stack or bottom wall already
        if (isConflict)
        {
            // We add the piece to stack
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece->GetCells()->Get(j, i) == true)
                    {
                        int realx = activePiece->GetPosition().x + j;
                        int realy = activePiece->GetPosition().y + i;
                        stackCells->Set(realx, realy, true);
                    }
                }
            }

            // Delete active piece, activate the waiting piece and generate new waiting piece
            delete activePiece;
            activePiece = waitingPiece;
            activePiece->Activate();
            waitingPiece = new Piece();
            waitingPiece->InitializeD2D(m_pRenderTarget);

            // If we have a collision right after we generate the new piece, 
            // it means the stack is too high, so game over
            if (activePiece->StackCollision(stackCells))
                gameOver = true;
        }
    }

    autoFallAccumulated2 += elapsedTime;
    if (autoFallAccumulated2 > autoFallDelay2) //여기
    {
        autoFallAccumulated2 = 0;

        // Remove any full rows
        int removed = stack->RemoveLines(stackCells2);
        if (removed > 0)
        {
            score += pow(2, removed) * 100;
            autoFallDelay2 = autoFallDelay2 * 0.98;
        }

        // Move down the active piece
        bool isConflict = activePiece2->Advance2(stackCells2);
        // If we have a conflict with the stack, it means we were sitting on the stack or bottom wall already
        if (isConflict)
        {
            // We add the piece to stack
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (activePiece2->GetCells2()->Get(j, i) == true)
                    {
                        int realx = activePiece2->GetPosition2().x + j;
                        int realy = activePiece2->GetPosition2().y + i;
                        stackCells2->Set(realx, realy, true);
                    }
                }
            }

            // Delete active piece, activate the waiting piece and generate new waiting piece
            delete activePiece2;
            activePiece2 = waitingPiece2;
            activePiece2->Activate();
            waitingPiece2 = new Piece();
            waitingPiece2->InitializeD2D(m_pRenderTarget);

            // If we have a collision right after we generate the new piece, 
            // it means the stack is too high, so game over
            if (activePiece2->StackCollision(stackCells2))
                gameOver2 = true;
        }
    }

}

HRESULT Engine::Draw()
{
    // This is the drawing method of the engine.
    // It runs every frame

    // Draws the elements in the game using Direct2D
    HRESULT hr;

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());


    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	
	// Below you can add drawing logic for your game elements
    
    stack->Draw(m_pRenderTarget);
    activePiece->Draw(m_pRenderTarget);
    waitingPiece->Draw(m_pRenderTarget);

    stack2->Draw2(m_pRenderTarget);
    activePiece2->Draw2(m_pRenderTarget);
    waitingPiece2->Draw2(m_pRenderTarget);
    DrawTextAndScore();

    hr = m_pRenderTarget->EndDraw();

    return S_OK;
}

void Engine::DrawTextAndScore()
{
    // Text and score
    int padding = (RESOLUTION_Y - (STACK_HEIGHT + 1) * CELL_SIZE) / 2;
    int centerRight = RESOLUTION_X - (RESOLUTION_X - padding - (STACK_WIDTH + 2) * CELL_SIZE) / 2;


    D2D1_RECT_F rectangle1 = D2D1::RectF(centerRight - 200, padding, centerRight + 200, padding + 100);
    m_pRenderTarget->DrawText(
        L"Next Piece",
        15,
        m_pTextFormat,
        rectangle1,
        m_pWhiteBrush
    );


    D2D1_RECT_F rectangle2 = D2D1::RectF(centerRight - 200, padding + 200, centerRight + 200, padding + 300);
    m_pRenderTarget->DrawText(
        L"Score",
        5,
        m_pTextFormat,
        rectangle2,
        m_pWhiteBrush
    );


    D2D1_RECT_F rectangle3 = D2D1::RectF(centerRight - 200, padding + 300, centerRight + 200, padding + 400);
    WCHAR scoreStr[64];
    swprintf_s(scoreStr, L"%d        ", score);
    m_pRenderTarget->DrawText(
        scoreStr,
        7,
        m_pTextFormat,
        rectangle3,
        m_pWhiteBrush
    );

    if (over == true) {
        D2D1_RECT_F rectangle4 = D2D1::RectF(RESOLUTION_X / 2 - 200, RESOLUTION_Y / 2 + 200, RESOLUTION_X / 2 + 200, RESOLUTION_Y / 2 - 200);
        m_pRenderTarget->DrawText(
            L"Game Over!!",
            15,
            m_pTextFormat,
            rectangle4,
            m_pWhiteBrush
        );
    }
}
