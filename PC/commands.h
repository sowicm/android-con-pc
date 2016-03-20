
#ifndef commands_h
#define commands_h

namespace commands
{
    enum commands { begin = 0
        , watchSreenBegin = 100
        , watchSreenEnd = 101
        , capture = 102
        , mouseMove = 1
        , lbutton = 2
        , upwheel = 3
        , mbutton = 4
        , downwheel = 5
        , rbutton = 6
        , lbuttondown = 7
        , lbuttonup = 8
        , rbuttondown = 9
        , rbuttonup = 10
        , sendString = 30
        , kdWin = 31
        , kdUp = 32
        , kdDown = 33
        , kdRight = 34
        , kdLeft = 35
        , kdEsc = 36
        , kdHome = 37
        , kdEnd = 38
        , kdEnter = 39
        , kdSpace = 40
        , CtrlAltDelete = 41
        , AltF4 = 42
        , shutdown = 60
        , switchUser = 61
        , logOff = 62
        , lock = 63
        , restart = 64
        , sleep = 65
        , hibernate = 66
        , run = 70
        , runWithNoWindow = 71
        , msgBox = 72
        , showText = 73
        , dark = 74
        , hideCursor = 75
        , multiCursor = 76
        , showTextEx = 77
        , showInNotepad = 78
    };
}

#endif