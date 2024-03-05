
struct ObjectivesManager {
	struct Objective {
		ObjectivesManager& manager;
		void complete() {
			manager.onComplete(*this);
		}
	};

	std::vector<Objective> getCurrent(const BlackBoard& bb) = 0;

};