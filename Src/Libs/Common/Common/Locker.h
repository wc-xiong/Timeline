#include <synchapi.h>

class CAutoSectionLocker final
{
public:
	explicit CAutoSectionLocker(LPCRITICAL_SECTION section) noexcept :mSection(section) { EnterCriticalSection(mSection); }
	~CAutoSectionLocker() { LeaveCriticalSection(mSection); }

	CAutoSectionLocker(const CAutoSectionLocker&) = delete;
	CAutoSectionLocker& operator=(const CAutoSectionLocker&) = delete;

private:
	LPCRITICAL_SECTION mSection;
};