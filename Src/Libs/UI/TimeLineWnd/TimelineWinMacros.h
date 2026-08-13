#pragma once
// Windows.h 将 GetObject 定义为 GetObjectA/W，破坏 ITimeLineLayer::GetObject
#ifdef GetObject
#undef GetObject
#endif
#ifdef GetMessage
#undef GetMessage
#endif
#ifdef SendMessage
#undef SendMessage
#endif
#ifdef CreateWindow
#undef CreateWindow
#endif
