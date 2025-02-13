

#include "scheduling.h"

using namespace std;


/* */
scheduling::scheduling(unordered_map<string, vehicle> list_veh, set<string>& list_veh_confirmation, intersection_client& localmap, const configuration& config, set<string>& list_veh_removal){
	timestamp = duration<double>(chrono::system_clock::now().time_since_epoch()).count();
	index_EVs.resize(localmap.get_laneIdEntry().size());
	for (const auto& element : list_veh){
		
		/* if the vehicle update is not older than update_expiration_delta seconds ago, include the vehicle in the schedule
		*  if the vehicle update is older than update_expiration_delta seconds ago, do not include the vehicle in the schedule
		*/
		if ( timestamp - element.second.get_curTime() <= config.get_expDelta()){

			veh_id.push_back(element.first);
			auto vehicle_index = (int)veh_id.size() - 1;
			veh_index[element.first] = vehicle_index;
			
			time.push_back(element.second.get_curTime());
			speed.push_back(element.second.get_curSpeed());
			acceleration.push_back(element.second.get_curAccel());
			lane_id.push_back(element.second.get_curLaneID());
			state.push_back(element.second.get_curState());

			departurePosition_index.push_back(element.second.get_departurePosition());
			if (state.back() == "EV" && element.second.get_curDistance() >= element.second.get_length()){
				distance.push_back(element.second.get_curDistance() - element.second.get_length());
			}
			else{
				distance.push_back(0.1);
			}
			clearance_time.push_back(-1);							// !!!
			est.push_back(-1);										// !!!
			st.push_back(element.second.get_actualST());
			et.push_back(element.second.get_actualET());
			dt.push_back(element.second.get_actualDT());

			access.push_back(element.second.get_access());
			link_priority.push_back(element.second.get_linkPriority());

			/* for those vehicles that got access from the previous schedules but did not confirm yet, please check */
			if (list_veh_confirmation.find(veh_id.back()) != list_veh_confirmation.end()){
				/* if CARMA Streets received the confirmation from the vehicle updates, it will remove the vehicle ID from the waiting list */
				if (access[access.size() - 1] == true){
					list_veh_confirmation.erase(veh_id.back());
				}
				/* if CARMA Streets did not receive the confirmation from the vehicle updates yet, it will consider the vehicle as a DV for now */
				else{
					state[state.size() - 1] = "DV";
					et[et.size() - 1] = timestamp;
					access[access.size() - 1] = true;
				}
			}

			/* estimate the vehicle location, speed, state, etc. at a given future timestamp! */
			if (time.back() < timestamp + config.get_schedulingDelta()){
				
				for (int i = 0; i < (int)element.second.get_futureInfo().size(); ++i){
					
					if (element.second.get_futureInfo()[i].timestamp >= timestamp + config.get_schedulingDelta() || 
						i == (int)element.second.get_futureInfo().size() - 1){

						
						time.back() = element.second.get_futureInfo()[i].timestamp;
						lane_id.back() = element.second.get_futureInfo()[i].lane_id;
						distance.back() = element.second.get_futureInfo()[i].distance;
						speed.back() = element.second.get_futureInfo()[i].speed;
						acceleration.back() = element.second.get_futureInfo()[i].acceleration;

						if (state.back() == "EV" && lane_id.back() != element.second.get_entryLaneID()){
							spdlog::critical("Scheduling Class Issue - The future lane id of vehicle {0} is not correct! Vehicle {0} is in EV state, but its lane ID in the future path is not its entry lane ID; cur_timestamp: {1}, cur_lane_id: {2}, future_path_lane_id: {3}, future_path_timestamp: {4}, entry_lane_id: {5}, link_id: {6}", veh_id.back(), element.second.get_curTime(), element.second.get_curLaneID(), time.back(), lane_id.back(),  element.second.get_entryLaneID(), element.second.get_linkID());
							continue;
						} 
						else {
							time.back() = element.second.get_futureInfo()[i].timestamp;
							lane_id.back() = element.second.get_futureInfo()[i].lane_id;
							distance.back() = element.second.get_futureInfo()[i].distance;
							speed.back() = element.second.get_futureInfo()[i].speed;
							acceleration.back() = element.second.get_futureInfo()[i].acceleration;
						} 
						
						if (state.back() == "RDV"){
							if (lane_id.back() != element.second.get_entryLaneID()){
								lane_id.back() = element.second.get_entryLaneID();
							}
						} else if (state.back() == "DV"){
							if (lane_id.back() == element.second.get_entryLaneID()){
								lane_id.back() = element.second.get_linkID();
								distance.back() += localmap.get_laneLength(lane_id.back());
							} else if (lane_id.back() != element.second.get_linkID()){
								state.back() = "LV";
								dt.back() = time.back();
							}
						}

						break;

					}
				}
			}


			/* if the vehicle is in the EV state, estimate its earliest stopping time */
			if (state.back() == "EV"){

				double dx1 = max(distance.back(), 0.1);
				double dx2 = -pow(speed.back(), 2) / (2 * element.second.get_decelMax());
				double dx3 = ((pow(localmap.get_laneSpeedLimit(lane_id.back()), 2) - pow(speed.back(), 2)) / (2 * element.second.get_accelMax())) - 
					(pow(localmap.get_laneSpeedLimit(lane_id.back()), 2) / (2 * element.second.get_decelMax()));
				double t_a;
				double t_d;
				double t_cr;
				double v_mid;
				if (dx1 < dx2){
					double d_emergency = -pow(speed.back(), 2) / (2 * dx1);
					v_mid = speed.back();
					t_a = 0.0;
					t_d = - speed.back() / d_emergency;
					t_cr = 0.0;
				} 
				else if (dx1 < dx3){
					v_mid = sqrt(element.second.get_decelMax() * ((2 * dx1 * element.second.get_accelMax()) + pow(speed.back(), 2)) / 
						(element.second.get_decelMax() - element.second.get_accelMax()));
					t_a = (v_mid - speed.back()) / element.second.get_accelMax();
					t_d = -v_mid / element.second.get_decelMax();
					t_cr = 0.0;
				}
				else{
					v_mid = localmap.get_laneSpeedLimit(lane_id.back());
					t_a = (v_mid - speed.back()) / element.second.get_accelMax();
					t_d = -v_mid / element.second.get_decelMax();
					t_cr = (dx1 - dx3) / v_mid;
				}
				est.back() = time.back() + max(t_a + t_cr + t_d, 0.01);

				double max_speed = localmap.get_laneSpeedLimit(lane_id.back());
				spdlog::info("Scheduling Class - vehicle {0}: cur_time = {1}, EST = {2}, difference = {3}", veh_id.back(), time.back(), est.back(), est.back() - time.back());
				spdlog::info("cur_timestamp = {0}, initial speed = {1}, initial distance = {2}, speed limit = {3}, v_mid = {4}, t_a = {5}, t_cr = {6}, t_d = {7}, dx1 = {8}, dx2 = {9}, dx3 = {10}", time.back(), speed.back(), distance.back(), max_speed, v_mid, t_a, t_cr, t_d, dx1, dx2, dx3);
			}


			/* calculate the clearance time required by the vehicle to pass through its associated connection link*/
			if (state.back() == "EV" || state.back() == "RDV") {
				if (pow(localmap.get_laneSpeedLimit(element.second.get_linkID()), 2) / (2 * element.second.get_accelMax()) < localmap.get_laneLength(element.second.get_linkID())){
					clearance_time.back() = (localmap.get_laneLength(element.second.get_linkID()) / localmap.get_laneSpeedLimit(element.second.get_linkID())) + 
						(localmap.get_laneSpeedLimit(element.second.get_linkID()) / (2 * element.second.get_accelMax()));
				} else{
					clearance_time.back() = sqrt((2 * localmap.get_laneLength(element.second.get_linkID())) / element.second.get_accelMax());
				}
				clearance_time.back() = ceil(clearance_time.back());
			}
			else if (state.back() == "DV"){
				double dx = (pow(localmap.get_laneSpeedLimit(element.second.get_linkID()), 2) - pow(speed.back(), 2)) / (2 * element.second.get_accelMax());
				if (distance.back() < dx){
					clearance_time.back() = (sqrt(pow(speed.back(), 2) + (2 * element.second.get_accelMax() * distance.back())) - speed.back()) / element.second.get_accelMax();
				} else{
					clearance_time.back() = ((localmap.get_laneSpeedLimit(element.second.get_linkID()) - speed.back()) / element.second.get_accelMax()) + 
						((distance.back() - dx) / localmap.get_laneSpeedLimit(element.second.get_linkID()));
				}
				clearance_time.back() = ceil(clearance_time.back());
			}


			/* create the EV, RDV, DV, and LV lists */
			if (state.back() == "EV"){
				for (int i = 0; i < (int)index_EVs.size(); ++i){
					if (lane_id.back() == localmap.get_laneIdEntry()[i]){
						index_EVs[i].push_back(vehicle_index);
						break;
					}
				}
			}
			else if (state.back() == "RDV"){
				index_RDVs.push_back(vehicle_index);
			}
			else if (state.back() == "DV"){
				index_DVs.push_back(vehicle_index);
			}
			else if (state.back() == "LV"){
				index_LVs.push_back(vehicle_index);
			}

			spdlog::info("Scheduling Class Vehicle Info Update before Schedule - timestamp = {0}, vehicle id = {1}, lane_id = {2}, state = {3}, speed = {4} m/s, distance = {5} m, est = {6}, access = {7}, departure position = {8}", time.back(), veh_id.back(), lane_id.back(), state.back(), speed.back(), distance.back(), est.back(), access.back(), departurePosition_index.back());

		}
		else{
			if (list_veh_confirmation.find(element.first) != list_veh_confirmation.end()){
				list_veh_confirmation.erase(element.first);
			}
			spdlog::info("Vehicle {0} is not added to the schedule as its update is more than {1} seconds old!", element.first, config.get_expDelta());
			list_veh_removal.insert(element.first);
		}
	}

	sort(index_DVs.begin(), index_DVs.end(), sorting<int>(departurePosition_index, "asc"));
	for (int n = 0; n < (int)index_DVs.size(); ++n){
		int vehicle_index = index_DVs[n];
		departurePosition_index[vehicle_index] = n + 1;
	}
	
	sort(index_RDVs.begin(), index_RDVs.end(), sorting<double>(st, "asc"));
	sort(index_RDVs.begin(), index_RDVs.end(), sorting<int>(departurePosition_index, "asc"));
	for (int n = 0; n < (int)index_RDVs.size(); ++n) {
		int vehicle_index = index_RDVs[n];
		departurePosition_index[vehicle_index] = (int)index_DVs.size() + n + 1;
	}
	
	for (int i = 0; i < (int)index_EVs.size(); ++i) {
		sort(index_EVs[i].begin(), index_EVs[i].end(), sorting<double>(distance, "asc"));
	}

}

string scheduling::toCSV() const {
	
	string schedule_info = "";
	schedule_info += to_string(timestamp) + ",";
	for (int i = 0; i < (int)veh_id.size(); ++i){
		schedule_info += veh_id[i] + ",";
		schedule_info += to_string(time[i]) + ",";
		schedule_info += to_string(speed[i]) + ",";
		schedule_info += to_string(acceleration[i]) + ",";
		schedule_info += lane_id[i] + ",";
		schedule_info += state[i] + ",";
		schedule_info += to_string(departurePosition_index[i]) + ",";
		schedule_info += to_string(distance[i]) + ",";
		schedule_info += to_string(clearance_time[i]) + ",";
		schedule_info += to_string(st[i]) + ",";
		schedule_info += to_string(et[i]) + ",";
		schedule_info += to_string(dt[i]) + ",";
		schedule_info += to_string(access[i]);
		// Added comma to separate vehicle entries
		if ( i != (int)veh_id.size()-1 ) {
			schedule_info += ",";
		}
	}

	return schedule_info;
}

double scheduling::get_timestamp() const {return timestamp;}

/* */
vector<string> scheduling::get_vehicleIdList() const {return veh_id;}

/* */
unordered_map<string, int> scheduling::get_vehicleIndexList() const {return veh_index;}

/* */
vector<double> scheduling::get_timeList() const {return time;}

/* */
vector<double> scheduling::get_speedList() const {return speed;}

/* */
vector<double> scheduling::get_accelList() const {return acceleration;}

/* */
vector<string> scheduling::get_laneIdList() const {return lane_id;}

/* */
vector<string> scheduling::get_stateList() const {return state;}

/* */
vector<int> scheduling::get_departPosIndexList() const {return departurePosition_index;}

/* */
vector<double> scheduling::get_distanceList() const {return distance;}

/* */
vector<double> scheduling::get_clearTimeList() const {return clearance_time;}

/* */
vector<double> scheduling::get_estList() const {return est;}

/* */
vector<double> scheduling::get_stList() const {return st;}

/* */
vector<double> scheduling::get_etList() const {return et;}

/* */
vector<double> scheduling::get_dtList() const {return dt;}

/* */
vector<vector<int>> scheduling::get_indexEVs() const {return index_EVs;}

/* */
vector<int> scheduling::get_indexRDVs() const {return index_RDVs;}

/* */
vector<int> scheduling::get_indexDVs() const {return index_DVs;}

/* */
vector<int> scheduling::get_indexLVs() const {return index_LVs;}

/* */
vector<bool> scheduling::get_accessList() const {return access;}

/* */
vector<int> scheduling::get_linkPriorityList() const {return link_priority;}

/* */
void scheduling::set_st(int v_index, double st_added){st[v_index] = st_added;}

/* */
void scheduling::set_et(int v_index, double et_added){et[v_index] = et_added;}

/* */
void scheduling::set_dt(int v_index, double dt_added){dt[v_index] = dt_added;}

/* */
void scheduling::set_departPosIndex(int v_index, int pos_index){departurePosition_index[v_index] = pos_index;}

/* */
void scheduling::set_access(int v_index, bool access_value){access[v_index] = access_value;}

/* */
void scheduling::set_timestamp(double timestamp){this->timestamp = timestamp;}
