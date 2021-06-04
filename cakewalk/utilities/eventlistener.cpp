#include "eventlistener.h"

#include "../core/interfaces.h"
#include "../features/misc.h"

void CEventListener::Setup(const std::deque<const char*>& arrEvents)
{
	if (arrEvents.empty())
		return;

	for (auto szEvent : arrEvents)
	{
		I::GameEvent->AddListener(this, szEvent, false);

		if (!I::GameEvent->FindListener(this, szEvent))
			throw std::runtime_error(XorStr("failed add listener"));
	}
}

void CEventListener::Destroy()
{
	I::GameEvent->RemoveListener(this);
}

void CEventListener::FireGameEvent(IGameEvent* pEvent)
{
	if (pEvent == nullptr)
		return;

	// get hash of event name
	const FNV1A_t uNameHash = FNV1A::Hash(pEvent->GetName());

	#pragma region eventlistener_handlers
	CMiscellaneous::Get().Event(pEvent, uNameHash);
	#pragma endregion
}
