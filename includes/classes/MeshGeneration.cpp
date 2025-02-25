#include "VoxelSystem.hpp"

/// Private functions

// Mesh Generation thread routine
void	VoxelSystem::_meshGenerationRoutine() {
	if (VERBOSE)
		cout << "Mesh Generation thread started" << endl;

	while (!_quitting) {

		// Check if there are meshes to generate, sleep if not
		if (!_requestedMeshesMutex.try_lock() || !_requestedMeshes.size()) {
			_requestedMeshesMutex.unlock();
			this_thread::sleep_for(chrono::milliseconds(THREAD_SLEEP_DURATION));
			continue;
		}


	}

	if (VERBOSE)
		cout << "Mesh Generation thread stopped" << endl;
}
/// ---

/// Public functions

// Request the update of a chunk mesh
// If a mesh is requested without a chunk, the chunk will be requested instead
void	VoxelSystem::requestMeshUpdate(const vector<ivec3> &Wpositions) {
	if (!Wpositions.size())
		return;

	_requestedMeshesMutex.lock();

	vector<ivec3>	chunkRequests;

	for (ivec3 pos : Wpositions) {
		if (!_chunks.count(pos))
			chunkRequests.push_back(pos);

		else if (find(_requestedMeshes.begin(), _requestedMeshes.end(), pos) == _requestedMeshes.end())
			_requestedMeshes.push_back(pos);
	}

	_requestedMeshesMutex.unlock();

	if (chunkRequests.size())
		requestChunk(chunkRequests);
}
/// ---