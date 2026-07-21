#pragma once
#include <opencv2/opencv.hpp>

namespace Theme {
    // --- Colors ---
    const cv::Scalar BoardLight(219, 235, 251);
    const cv::Scalar BoardDark(124, 163, 206);
    const cv::Scalar Background(110, 110, 110);
    const cv::Scalar TextDark(40, 40, 40);
    const cv::Scalar TextLight(255, 255, 255);
    const cv::Scalar TextMuted(100, 100, 100);
    const cv::Scalar TextRed(10, 10, 200);
    
    const cv::Scalar TableBorder(0, 0, 0);
    const cv::Scalar LineLight(230, 230, 230);
    const cv::Scalar OverlayDark(0, 0, 0);
    const cv::Scalar ButtonBg(220, 220, 220);

    const cv::Scalar HighlightEnemy(100, 100, 255);
    const cv::Scalar HighlightEmpty(150, 220, 150);
    const cv::Scalar HighlightSelected(255, 200, 100);

    const cv::Scalar RestBarColor(90, 200, 90, 255);
    const cv::Scalar RestBarTrack(40, 40, 40, 255);

    // --- Math & Layout ---
    constexpr double Pi = 3.14159265358979323846;
    constexpr int BoardCells = 8;
    constexpr int BoardOutlineThickness = 2;

    constexpr int RestBarHeight = 5;
    constexpr int RestBarInset = 4;
    constexpr long CooldownLong = 2000;
    constexpr long CooldownShort = 1000;
    
    // --- UI Tables ---
    constexpr int TableMaxMoves = 14;
    constexpr int TableRowHeight = 24;
    constexpr int TableHeaderHeight = 30;
    constexpr int TableMinW = 60;
    constexpr int TableMaxW = 220;
    constexpr int TableMaxH = 380;
    constexpr int TableDividerX = 100;
    constexpr int MoveColOffset = 115;
    constexpr int TimeColOffset = 8;
    constexpr int RowStartYOffset = 22;

    // --- Paddings ---
    constexpr int PadSmall = 5;
    constexpr int PadMedium = 10;
    constexpr int PadLarge = 15;
    constexpr int PadExtra = 20;
    constexpr int PadHuge = 25;

    // --- Buttons ---
    constexpr int BtnWidth = 200;
    constexpr int BtnHeight = 50;
    constexpr int BtnOffsetY = 60;

    // --- Fonts & Typography ---
    constexpr double FontTiny = 0.4;
    constexpr double FontSmall = 0.45;
    constexpr double FontMedium = 0.55;
    constexpr double FontLarge = 0.6;
    constexpr double FontHuge = 1.3;

    constexpr int ThickNormal = 1;
    constexpr int ThickBold = 2;
    constexpr int ThickTitle = 5;

    // --- Alpha Blending ---
    constexpr double AlphaBase = 0.4;
    constexpr double AlphaOverlay = 0.6;

    // --- Window & App Settings ---
    const std::string WindowName = "Kung Fu Chess";
    const std::string AssetsPath = "../assets/";
    constexpr int WindowWidth = 1024;
    constexpr int WindowHeight = 720;
    constexpr int BoardStartX = 260;
    constexpr int BoardStartY = 100;
    constexpr int UIOffsetX = 260;
    constexpr int UIOffsetY = 100;
    
    // --- Engine Settings ---
    constexpr int FpsDelayMs = 16;       // ~60 FPS (1000ms / 60)
    constexpr int UiRefreshDelayMs = 1; 
    constexpr int KeyEsc = 27;
}