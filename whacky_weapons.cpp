/*
	Agatha, 2020�2021

	MIT License.

	Explicitly: feel free to use this plugin, including with your modifications, on any server.

	##############

	Huge thanks go to . . . an individual who wishes to remain anonymous, but who helped me
	immeasurably in writing the code.  He insisted that I take all the credit, but that's not fair,
	so the least I can do is give a shoutout here.  Thank you so much!  This would not have been
	possible without you.

	Also thanks to GEP for helping me debug and test, various on #bzflag for hints, and several
	others for listing to my ideas and complaints and providing feedback and suggestions.  Thank you
	also to the forums for encouragement, discouragement, digressions, and outright insults. <3

	To the many *many* people who said that what I wanted to do was flat out impossible . . . well, 
	you've got to try!!!
 */

#include <cassert>
#include <cmath>

#include <algorithm>

#include <array>
#include <map>
#include <set>
#include <vector>

#if 0//def __INTELLISENSE__
	// I do plugin development in a different directory from compiling it because I am a horrible
	// person.
	#include "../../bzflag/include/Address.h"
	#include "../../bzflag/include/bzfsAPI.h"
	#include "../../bzflag/include/Pack.h"
	#include "../../bzflag/include/Protocol.h"
	#include "../../bzflag/include/vectors.h"
	#include "../../bzflag/src/bzfs/bzfs.h"
#else
	#include "bzfsAPI.h"
	//#include "plugin_utils.h"
	#include "../src/bzfs/bzfs.h"
#endif

#define PI 3.14159265f

inline static float random_float() {
	return (float)rand() / (float)RAND_MAX;
}
inline static float random_float(float low,float high) {
	return low + (high-low)*random_float();
}

inline static float to_radians(float angle_in_degrees) {
	return angle_in_degrees * (PI/180.0f);
}

static std::string exec(char const* cmd) {
	//https://stackoverflow.com/a/478960
	std::array<char,128> buffer;
	std::string result;
	#ifdef _WIN32
	std::unique_ptr<FILE,decltype(&_pclose)> pipe(_popen(cmd,"r"),_pclose);
	if (!pipe) throw std::runtime_error("_popen() failed!");
	#else
	std::unique_ptr<FILE,decltype(&pclose)> pipe(popen(cmd,"r"),pclose);
	if (!pipe) throw std::runtime_error("popen() failed!");
	#endif
	while (fgets(buffer.data(),buffer.size(),pipe.get())!=nullptr) {
		result += buffer.data();
	}
	return result;
}

/*static fvec3 rotate_vector_around_z(fvec3 const& vec, float angle_in_radians) {
	float c=std::cos(angle_in_radians), s=std::sin(angle_in_radians);
	return fvec3(
		c*vec.x - s*vec.y,
		s*vec.x + c*vec.y,
		vec.z
	);
}*/

inline static bool version_is_parsable_and_at_least( bz_ApiString const& str, int ver_maj,int ver_min,int ver_rev ) {
	int ver_maj2, ver_min2, ver_rev2;
	if (sscanf( str.c_str(), "%d.%d.%d", &ver_maj2,&ver_min2,&ver_rev2 )==3) {
		bz_debugMessagef(2,"Parsed version string \"%s\" as %d.%d.%d\n",str.c_str(),ver_maj2,ver_min2,ver_rev2);

		if      (ver_maj2<ver_maj) return false;
		else if (ver_maj2>ver_maj) return true;

		if      (ver_min2<ver_min) return false;
		else if (ver_min2>ver_min) return true;

		if      (ver_rev2<ver_rev) return false;
		#if 0
		else if (ver_rev2>ver_rev) return true;

		return true;
		#else //optimization
		return true;
		#endif
	}
	else return false;
}

inline static std::string get_player_flag_abbrev(bz_BasePlayerRecord* player) {
	// Note: `player->currentFlag` is e.g. "Nyan Cat (+NC)" whereas this is just "NC".
	char const* current_flag = bz_getPlayerFlag(player->playerID);
	if (current_flag==nullptr) return "";
	else                       return current_flag;
}

static bz_eTeamType const rainbow_colors[] = {
	eRedTeam, eHunterTeam, eRogueTeam, eGreenTeam, eBlueTeam, ePurpleTeam
};

/*enum class SHOT {
	TN
};*/

// This copies the memory layout of Shots::Manager so that we can get LastGUID.
#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Winvalid-offsetof" //GCC thinks we're doing something weird.  It's right.
#endif
class ShotManagerMemoryLayout { public:
	virtual ~ShotManagerMemoryLayout() = default;
	static double _a;
	Shots::ShotEvent _b, _c;
	Shots::ShotList _d, _e;
	Shots::FlightLogicMap _f;
	uint32_t LastGUID;
};
#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif

class WhackyWeapons final : public bz_Plugin, public bz_CustomSlashCommandHandler {
	private:
		std::vector<bz_BasePlayerRecord*> players;
		std::map<int,double> players_last_update_times;

		#if 0
		#ifdef __GNUC__
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wshadow"
		#endif
		class ShotRecord final {
			public:
				SHOT type;

				int player_id;
				int shot_id;
				bz_eTeamType color;

				double start_time;
				fvec3 last_pos; double last_time;
				fvec3 pos; double time;

				fvec3 vel;

				int num_updates = 0;

				ShotRecord(SHOT type, int player_id,int shot_id,bz_eTeamType color, double start_time) :
					type(type), player_id(player_id),shot_id(shot_id),color(color), start_time(start_time)
				{
					// Note cannot use ShotManager here since bullet hasn't been added until
					// bz_eShotFiredEvent returns

					//printf("Adding special shot %d from %d at %f\n",shot_id,player_id,start_time);
				}

				bool update() {
					//See src/bzfs/bzfs.h 161
					Shots::ShotRef shot = ShotManager.FindShot(shot_id);

					if (shot!=nullptr) {
						if (shot->LastUpdateTime>last_time) {
							++num_updates;
							if (num_updates>1) {
								last_pos  = pos;
								last_time = time;
							}
							pos  = shot->LastUpdatePosition;
							time = shot->LastUpdateTime;
							if (num_updates>1) {
								vel = (pos-last_pos) / (time-last_time);
							}
						}
						return true;
					} else {
						return false;
					}
				}
		};
		#ifdef __GNUC__
			#pragma GCC diagnostic pop
		#endif
		std::set<ShotRecord*> special_shots;
		#endif

		double nc_nextfiretime = -1.0;

		std::string plugin_name;

		/*struct TridentRecord final { double splittime; ShotRecord* rec; };
		std::set<struct TridentRecord> tn_bulletsneedingsplit;*/

	public:
		WhackyWeapons() {
			//std::string build = exec("git rev-parse HEAD");
			std::string build = exec("git show --pretty=\"format:%h at %ci\" --no-patch HEAD");
			if (build.empty()) build="(unknown build)";
			plugin_name = "Whacky Weapons Plugin - Agatha 2021 - " + build;
		}

		virtual char const* Name() override {
			return plugin_name.c_str();
		}
		virtual void Init(char const* /*config*/) override {
			Register(bz_ePlayerJoinEvent  );
			Register(bz_eTickEvent        );
			Register(bz_eShotFiredEvent   );
			Register(bz_ePlayerUpdateEvent);
			Register(bz_ePlayerDieEvent   );

			bz_registerCustomSlashCommand("smite",this);

			{
				//Anti-Aircraft (AA)
				bz_RegisterCustomFlag("AA", "Anti-Aircraft", "Bullet fires upward.  Move forward/backward to aim.", 0, eGoodFlag);
				bz_registerCustomBZDBDouble("_wwAAdeflect",   0.3, 0, false);
				bz_registerCustomBZDBDouble("_wwAAvelfactor", 2.0, 0, false);

				//Ball Lightning (BL)
				//bz_RegisterCustomFlag("BL", "Ball Lightning", "Creates a lightning ball that zaps enemies.", 0, eGoodFlag);

				//Flamethrower (FL)
				//bz_RegisterCustomFlag("FL", "Flamethrower", "Shooting creates a small patch of fire that persists for some time.", 0, eGoodFlag);

				//Fractal (FR)
				//bz_RegisterCustomFlag("FR", "Fractal", "Bullet splits repeatedly.  Long reload.", 0, eGoodFlag);

				//Girl Scout (GS)
				//bz_RegisterCustomFlag("GS", "Girl Scout", "Bullets are cookies.  When tank gets hit by a cookie, gains obesity flag and pays girl scout 1 point.", 0, eGoodFlag);

				//Hook Shot (HS)
				//bz_RegisterCustomFlag("HS", "Hook Shot", "Bullet turns left or right after traveling forward.", 0, eGoodFlag);

				//Lightning Bolt (LB)
				bz_RegisterCustomFlag("LB", "Lightning Bolt", "Shooting instead triggers a lightning bolt.  Don't stand still when firing, or you'll hit yourself!", 0, eGoodFlag);
				bz_registerCustomBZDBDouble("_wwLBstartheight", 100.0, 0, false);
				bz_registerCustomBZDBDouble("_wwLBfactor",        5.0, 0, false);
				bz_registerCustomBZDBDouble("_wwLBradius",        1.0, 0, false);
				bz_registerCustomBZDBDouble("_wwLBradiussmite",   4.0, 0, false);

				//Mortar (MR)
				//bz_RegisterCustomFlag("MR", "Mortar", "Bullet is launched forward and up, following a thrown arc, exploding when it lands.", 0, eGoodFlag);

				//Nyan Cat (NC)
				//	TODO: Maybe consider that Nyan Cat's tempo is 142 beats/minute -> 0.4225 seconds per beat?
				bz_RegisterCustomFlag("NC", "Nyan Cat", "Tank uncontrollably spews rainbow bullets backward, and can't fire forward.", 0, eGoodFlag);
				bz_registerCustomBZDBDouble("_wwNCrate",        0.03, 0, false);
				bz_registerCustomBZDBDouble("_wwNCvelfactor",   1.00, 0, false);
				bz_registerCustomBZDBDouble("_wwNCspreadangle", 0.50, 0, false);

				//Orion Drive (OD)
				//bz_RegisterCustomFlag("OD", "Orion Drive", "Shooting triggers an explosion that yeets your tank forward.", 0, eGoodFlag);

				//Perpendicular Laser (PL)
				//bz_RegisterCustomFlag("PL", "Perpendicular Laser", "Laser starts at first bounce, and comes out perpendicular to the surface instead.", 0, eGoodFlag);

				//Phoenix (PX)
				//bz_RegisterCustomFlag("PX", "Phoenix", "You respawn where you last died, creating a shockwave.", 0, eGoodFlag);

				//Railgun (RA)
				//bz_RegisterCustomFlag("RA", "Railgun", "Fires a very fast projectile through walls that leaves a destructive sonic boom.  Long reload.", 0, eGoodFlag);

				//Rico Laser (RL)
				//bz_RegisterCustomFlag("RL", "Rico Laser", "Laser only exists after first bounce.", 0, eGoodFlag);

				//Snowball Inhibitor
				//bz_RegisterCustomFlag("SI", "Snowball Inhibitor", "Fires a snowball that freezes the enemy tank's gun.", 0, eGoodFlag); //rabbit shots

				//Sidewinder Missiles (SM)
				bz_RegisterCustomFlag("SM", "Sidewinder Missiles", "Fires two guided missiles sideways instead of a single missile forward", 0, eGoodFlag);

				//Snake Shot (SS)
				//bz_RegisterCustomFlag("SS", "Snake Shot", "Bullet takes a wavy path.", 0, eGoodFlag);

				//Temporal Displacement (TD)
				//bz_RegisterCustomFlag("TD", "Temporal Displacement", "Bullet appears some time after it was fired.", 0, eGoodFlag);
				//bz_registerCustomBZDBDouble("_wwTDtime", 2.0, 0, false);

				//Trident (TN)
				//bz_RegisterCustomFlag("TN", "Trident", "Super bullet that splits into three.", 0, eGoodFlag);
				//bz_registerCustomBZDBDouble("_wwTNsplittime", 2.0, 0, false);
			}

			// Tell the server not to wait for stuff, and to process as fast as possible!  This is
			// important for consistent tick-based firing behavior.  Note must be positive to be
			// recognized by bzfs
			MaxWaitTime = 0.000001f;
		}
		virtual void Cleanup() override {
			Flush();

			bz_removeCustomSlashCommand("smite");

			bz_removeCustomBZDBVariable("_wwAAdeflect"  );
			bz_removeCustomBZDBVariable("_wwAAvelfactor");

			bz_removeCustomBZDBVariable("_wwLBstartheight");
			bz_removeCustomBZDBVariable("_wwLBfactor"     );
			bz_removeCustomBZDBVariable("_wwLBradius"     );
			bz_removeCustomBZDBVariable("_wwLBradiussmite");

			bz_removeCustomBZDBVariable("_wwNCrate"       );
			bz_removeCustomBZDBVariable("_wwNCvelfactor"  );
			bz_removeCustomBZDBVariable("_wwNCspreadangle");

			//bz_removeCustomBZDBVariable("_wwTDtime");

			//bz_removeCustomBZDBVariable("_wwTNsplittime");
		}
		virtual void Event(bz_EventData* event_data) override {
			size_t num_players = (size_t)bz_getPlayerCount();
			players.resize(num_players);
			//printf("Players:\n");
			for (size_t i=0;i<num_players;++i) {
				players[i] = bz_getPlayerByIndex((int)i);
				//printf( "  Player %zu/%zu: ID %d 0x%p\n", i,num_players, players[i]!=nullptr?players[i]->playerID:-1, (void*)players[i] );
			}

			double now = bz_getCurrentTime();

			float bzdb_shot_speed = static_cast<float>(bz_getBZDBDouble("_shotSpeed"));

			switch (event_data->eventType) {
				//Note events should have been registered above

				case bz_ePlayerJoinEvent: {
					bz_PlayerJoinPartEventData_V1* data = static_cast<bz_PlayerJoinPartEventData_V1*>(event_data);

					// Ensure that the client has version 2.4.22 or newer.  This is required for the
					// cancel_firing_shot member function below.
					bz_debugMessagef( 2, "Player \"%s\" (%d) sent version string: \"%s\"\n", data->record->callsign.c_str(),data->playerID, data->record->clientVersion.c_str() );
					if (!version_is_parsable_and_at_least( data->record->clientVersion, 2,4,22 )) {
						if (data->record->team==bz_eTeamType::eObservers) {
							std::string msg =
								"WARNING: This server has a plugin that requires bzflag version 2.4.22 or newer to play.  Your version \""+
								(std::string)data->record->clientVersion+
								"\" is unparseable or less than this.  Please update your bzflag if you would like to play!"
							;
							bz_sendTextMessage(BZ_SERVER,data->playerID,bz_eMessageType::eActionMessage,msg.c_str());
						} else {
							std::string msg =
								"ERROR: This server has a plugin that requires bzflag version 2.4.22 or newer to play.  Your version \""+
								(std::string)data->record->clientVersion+
								"\" is unparseable or less than this.  Please update your bzflag and try again (or you can join as an observer)!"
							;
							bz_sendTextMessage(BZ_SERVER,data->playerID,bz_eMessageType::eActionMessage,msg.c_str());

							bz_kickUser(data->playerID,"bzflag version unknown or too old",false);
						}
						bz_sendTextMessagef(
							BZ_SERVER,BZ_ADMINCHAT,bz_eMessageType::eChatMessage,
							"Player \"%s\" tried to join, but has a client that is too old.%s", data->record->callsign.c_str(),
							data->record->team==bz_eTeamType::eObservers ? "" : "  They were yeeted."
						);
					}

					break;
				}

				case bz_eTickEvent: {
					//bz_TickEventData_V1* data = static_cast<bz_TickEventData_V1*>(event_data);

					#if 0
					//printf("Updating.  There are %zu -> ",special_shots.size());
					for ( auto iter=special_shots.begin(); iter!=special_shots.end(); ) {
						bool still_exists = (*iter)->update();
						if (!still_exists) {
							auto iter2=iter; ++iter2; delete *iter; special_shots.erase(iter); iter=iter2;
						} else {
							++iter;
						}
					}
					//printf("%zu special shots!\n",special_shots.size());
					#endif

					//Nyan Cat
					if ( nc_nextfiretime==-1.0 || now-nc_nextfiretime>5.0 ) {
						//Do nothing (i.e., only set it if it's the first time, or if it's too large)
						nc_nextfiretime = now;
					} else {
						double rate         =                    bz_getBZDBDouble("_wwNCrate");
						float  velfactor    = static_cast<float>(bz_getBZDBDouble("_wwNCvelfactor"));
						float  shot_speed   = bzdb_shot_speed * velfactor;
						float  spread_angle = static_cast<float>(bz_getBZDBDouble("_wwNCspreadangle"));

						//printf("Nyan Cat %f -> %f (speed %f)\n",nc_nextfiretime,now,(double)shot_speed);

						while (nc_nextfiretime+rate<=now) {
							for (size_t i=0;i<num_players;++i) {
								//This should not be necessary, but when a client quits abruptly it seems to happen . . .
								if (players[i]==nullptr) continue;

								std::string current_flag = get_player_flag_abbrev(players[i]);
								if (current_flag=="NC") {
									fvec3 player_pos, player_vel; float player_rotation;
									estimate_player_state_at_time( players[i], nc_nextfiretime, &player_pos,&player_vel,&player_rotation );

									for (int j=0;j<6;++j) {
										float shot_angle = player_rotation + PI;
										//shot_angle += 0.5f*PI * random_float() - 0.25f*PI;
										//shot_angle += to_radians(random_float( -0.5f, 0.5f ));
										shot_angle += to_radians(random_float( -spread_angle, spread_angle ));
										float vec[3]; bz_vectorFromRotations(0.0f,shot_angle,vec);
										fvec3 shot_vec = {vec[0],vec[1],vec[2]};
										fvec3 shot_vel = shot_speed * shot_vec;

										double time_ago = now - nc_nextfiretime;
										//printf("  Firing %f ago . . .\n",time_ago);

										fvec3 shot_pos = player_pos;
										shot_pos += 5.0f * shot_vec;
										shot_pos += static_cast<float>(time_ago) * shot_vel;
										shot_pos.z += 5.0f - (float)j;

										fire_shot(
											players[i]->playerID, rainbow_colors[j], "NC",
											shot_pos, (player_vel+shot_vel)/bzdb_shot_speed
										);

										/*printf("Nyan Cat fire %d, %d, %.3f %.3f %.3f -> %.3f %.3f %.3f!\n",
											player->playerID, (int)rainbow_colors[nc_shotcolorindex],
											(double)shot_pos.x,(double)shot_pos.y,(double)shot_pos.z,
											(double)shot_vel.x,(double)shot_vel.y,(double)shot_vel.z
										);*/
									}
								}
							}

							nc_nextfiretime += rate;

							//printf("Nyan Cat step!\n");
						}
					}

					#if 0
					double tn_splittime = bz_getBZDBDouble("_wwTNsplittime" );
					for ( auto iter=special_shots.begin(); iter!=special_shots.end(); ) {
						ShotRecord* shot_rec = *iter;

						double time_flying = shot_rec->time - shot_rec->start_time;

						switch (shot_rec->type) {
							case SHOT::TN: {
								//Trident
								printf("  time %f: %f,%f -> %f cmp %f\n",now,shot_rec->time,shot_rec->start_time,time_flying,tn_splittime);
								if ( time_flying > tn_splittime ) {
									Shots::ShotRef shot = ShotManager.FindShot(shot_rec->shot_id);

									
									fvec3 split_pos = shot->LastUpdatePosition + (now-shot->LastUpdateTime)*shot->vel;



									fvec3 split_pos = shot->pos - static_cast<float>(time_flying-tn_splittime)*shot->vel;

									fvec3 vel_l = rotate_vector_around_z(shot->vel,to_radians( 45.0f));
									fvec3 vel_r = rotate_vector_around_z(shot->vel,to_radians(-45.0f));

									fire_shot( shot_rec->player_id, shot_rec->color, split_pos,vel_l/bzdb_shot_speed );
									fire_shot( shot_rec->player_id, shot_rec->color, split_pos,vel_r/bzdb_shot_speed );

									goto REMOVE_SPECIAL_SHOT;
								}

								break;
							}
						}
						++iter; continue;

						REMOVE_SPECIAL_SHOT:
						auto iter2=iter; ++iter2; delete *iter; special_shots.erase(iter); iter=iter2;
					}
					#endif

					break;
				}

				case bz_eShotFiredEvent: {
					bz_ShotFiredEventData_V1* data = static_cast<bz_ShotFiredEventData_V1*>(event_data);

					bz_BasePlayerRecord* player = nullptr;
					for (bz_BasePlayerRecord* player2 : players) {
						if (player2!=nullptr&&player2->playerID==data->playerID) { player=player2; break; }
					}
					assert(player!=nullptr);

					std::string current_flag = get_player_flag_abbrev(player);
					if (current_flag.empty()) break;

					//int player_id = data->playerID;
					//bz_eTeamType player_color = bz_getPlayerTeam(data->playerID);
					bz_eTeamType player_color = player->team;

					/*int shot_id_that_will_be_created;
					{
						// See ShotManager.cxx 143.  Ideally, we would work off of
						// ShotManager->LastGUID, but that's private so we can't :V
						// Instead we cheat and inspect it by dereferencing the memory with pointer
						// hacks.
						#ifdef __GNUC__
							#pragma GCC diagnostic push
							#pragma GCC diagnostic ignored "-Winvalid-offsetof" //GCC thinks we're doing something weird.  It's right.
						#endif
						uint32_t LastGUID = *reinterpret_cast<uint32_t*>(
							reinterpret_cast<char*>(&ShotManager) +
							offsetof(ShotManagerMemoryLayout,LastGUID) //offsetof(Shots::Manager,LastGUID)
						);
						#ifdef __GNUC__
							#pragma GCC diagnostic pop
						#endif
						LastGUID++;
						if (LastGUID==INVALID_SHOT_GUID) LastGUID=INVALID_SHOT_GUID+1;
						shot_id_that_will_be_created = LastGUID;
					}*/

					fvec3 player_pos, player_vel; float player_rotation;
					estimate_player_state_at_time( player, nc_nextfiretime, &player_pos,&player_vel,&player_rotation );

					if      (current_flag=="AA") {
						cancel_firing_shot(data);

						float deflect   = static_cast<float>(bz_getBZDBDouble("_wwAAdeflect"  ));
						float velfactor = static_cast<float>(bz_getBZDBDouble("_wwAAvelfactor"));

						fvec3 shot_vec = fvec3( deflect*fvec2(player_vel[0],player_vel[1]), 1.0f );
						shot_vec = shot_vec.normalize();

						fvec3 shot_vel = (velfactor*bzdb_shot_speed) * shot_vec;
						fire_shot(
							player->playerID, player_color, "AA",
							player_pos+fvec3(0.0f,0.0f,5.0f), (player_vel+shot_vel)/bzdb_shot_speed
						);
					}
					/*else if (current_flag=="BL") {
						//TODO
					}
					else if (current_flag=="FL") {
						cancel_firing_shot(data);
						//TODO
					}
					else if (current_flag=="FR") {
						//TODO
					}
					else if (current_flag=="GS") {
						//TODO
					}
					else if (current_flag=="HS") {
						//TODO
					}*/
					else if (current_flag=="LB") {
						cancel_firing_shot(data);

						float factor = static_cast<float>(bz_getBZDBDouble("_wwLBfactor"));
						float radius = static_cast<float>(bz_getBZDBDouble("_wwLBradius"));
						fvec3 pos = player_pos+fvec3(factor*fvec2(player_vel[0],player_vel[1]),0.0f);
						fire_smite(player->playerID,fvec3(pos.xy(),0.0f),"LB",radius);
					}
					/*else if (current_flag=="MR") {
						//TODO
					}*/
					else if (current_flag=="NC") {
						cancel_firing_shot(data);
					}
					/*else if (current_flag=="OD") {
						//TODO
					}
					else if (current_flag=="PL") {
						//TODO
					}
					else if (current_flag=="PX") {
						//TODO
					}
					else if (current_flag=="RA") {
						cancel_firing_shot(data);

						float hsize = static_cast<float>( 0.5 * bz_getBZDBDouble("_worldSize") );

						fvec3 pos = player_pos;
						fvec3 dir = player_vel; dir.normalize();
						pos += 15.0f*dir;
						while ( pos[0]>-hsize && pos[0]<hsize && pos[1]>-hsize && pos[1]<hsize ) {
							fire_sw( player->playerID, player_color, pos );
							pos += 5.0f * dir;
						}

						//TODO
					}
					else if (current_flag=="RL") {
						//TODO
					}
					else if (current_flag=="SI") {
						//TODO
					}*/
					else if (current_flag=="SM") {
						cancel_firing_shot(data);

						float vec_l[3]; bz_vectorFromRotations(0.0f,player_rotation+0.5f*PI,vec_l);
						float vec_r[3]; bz_vectorFromRotations(0.0f,player_rotation-0.5f*PI,vec_r);
						fvec3 shot_vec_l = {vec_l[0],vec_l[1],vec_l[2]};
						fvec3 shot_vec_r = {vec_r[0],vec_r[1],vec_r[2]};
						fvec3 shot_vel_l = shot_vec_l + player_vel;
						fvec3 shot_vel_r = shot_vec_r + player_vel;

						//TODO: crashes the server for some reason
						fire_gm( player->playerID, player_color, "SM", player_pos, shot_vel_l/bzdb_shot_speed, -1 );
						fire_gm( player->playerID, player_color, "SM", player_pos, shot_vel_r/bzdb_shot_speed, -1 );
					}
					/*else if (current_flag=="SS") {
						//TODO
					}
					else if (current_flag=="TD") {
						//TODO
					}
					else if (current_flag=="TN") {
						//TODO
						//special_shots.insert(new ShotRecord( SHOT::TN, player->playerID,shot_id_that_will_be_created,player_color, data->eventTime ));
					}*/

					break;
				}

				case bz_ePlayerUpdateEvent: {
					bz_PlayerUpdateEventData_V1* data = reinterpret_cast<bz_PlayerUpdateEventData_V1*>(event_data);
					players_last_update_times[data->playerID] = data->stateTime;
					break;
				}

				case bz_ePlayerDieEvent: {
					bz_PlayerDieEventData_V1* data = reinterpret_cast<bz_PlayerDieEventData_V1*>(event_data);

					uint32_t shot_guid = bz_getShotGUID(data->killerID, data->shotID);

					if (bz_shotHasMetaData(shot_guid,"wwType")) {
						data->killerID = bz_getShotMetaDataI(shot_guid, "owner");

						std::string weapon = bz_getShotMetaDataS(shot_guid, "wwType");
						if (weapon=="smite") {
							bz_sendTextMessagef(BZ_SERVER,data->playerID,bz_eMessageType::eChatMessage,"You have been smitten by %s.",bz_getPlayerCallsign(data->killerID));
						}
					}

					break;
				}

				default: break;
			}

			for (bz_BasePlayerRecord* player : players) if (player!=nullptr) bz_freePlayerRecord(player);
		}
		virtual bool SlashCommand(int from_player_id, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params) {
			if (command=="smite") {
				bz_BasePlayerRecord* player = bz_getPlayerByIndex(from_player_id);

				if (player->admin) {
					std::string target_id = params->get(0); //Note safely returns "" if parameter doesn't exist.
					//printf("Target ID: \"%s\"",target_id.c_str());
					bz_BasePlayerRecord* target = bz_getPlayerBySlotOrCallsign(target_id.c_str());
					if (target) {
						// Note we're *attempting* to smite; the smite might miss (e.g. if the target is
						// protected from above.
						std::string msg = (std::string)player->callsign + " is smiting " + (std::string)target->callsign + "!";
						bz_sendTextMessage(BZ_SERVER,BZ_ALLUSERS,bz_eMessageType::eActionMessage,msg.c_str());

						fvec3 target_pos, target_vel; float target_rot;
						estimate_player_state_at_time(target,bz_getCurrentTime()+(double)target->lag*0.001,&target_pos,&target_vel,&target_rot);
						float radius = static_cast<float>(bz_getBZDBDouble("_wwLBradiussmite"));
						fire_smite(from_player_id,fvec3(target_pos.xy(),0.0f),"smite",radius);

						bz_freePlayerRecord(target);
					} else {
						bz_sendTextMessagef(BZ_SERVER,from_player_id,bz_eMessageType::eActionMessage,"Error: player \"%s\" not found.",target_id.c_str());
					}
				} else {
					bz_sendTextMessage(BZ_SERVER,from_player_id,bz_eMessageType::eActionMessage,"Error: only admins can run /smite.");
				}

				bz_freePlayerRecord(player);

				return true;
			}

			return false;
		}

		void estimate_player_state_at_time( bz_BasePlayerRecord const* player, double time_now, fvec3* player_pos,fvec3* player_vel,float* player_rotation ) {
			// Note: player->lastUpdateTime is wrong!  See bzfsAPI.cxx 932 it's the message time.
			// I think this is a bug.  Anyway we have to keep track of the last updated time
			// ourselves :/
			float dt = static_cast<float>( time_now - players_last_update_times[player->playerID] );

			player_pos->x = player->lastKnownState.pos[0];
			player_pos->y = player->lastKnownState.pos[1];
			player_pos->z = player->lastKnownState.pos[2];

			player_vel->x = player->lastKnownState.velocity[0];
			player_vel->y = player->lastKnownState.velocity[1];
			player_vel->z = player->lastKnownState.velocity[2];

			*player_pos += dt * *player_vel;

			*player_rotation = player->lastKnownState.rotation + dt*player->lastKnownState.angVel;

			/*printf("%f -> %f (%f): %.3f+%.3f, %.3f+%.3f, %.3f+%.3f | %.3f+%.3f\n",
				player->lastUpdateTime, time_now, (double)dt,
				player->lastKnownState.pos[0], player->lastKnownState.velocity[0],
				player->lastKnownState.pos[1], player->lastKnownState.velocity[1],
				player->lastKnownState.pos[2], player->lastKnownState.velocity[2],
				player->lastKnownState.rotation, player->lastKnownState.angVel
			);*/
		}

		uint32_t fire_something( int from_player_id, bz_eTeamType color, char const* internal_type,char const* ww_type, fvec3 const& pos,fvec3 const& vel_dived_by_bzdb_shotspeed, int at_player_id=-1 ) {
			float origin[3]={pos.x,pos.y,pos.z}, vector[3]={vel_dived_by_bzdb_shotspeed.x,vel_dived_by_bzdb_shotspeed.y,vel_dived_by_bzdb_shotspeed.z};
			//`vector` times BZDB shot speed is the velocity.  See WorldWeapons.cxx line 47.
			uint32_t shot_guid = bz_fireServerShot( internal_type, origin,vector, color, at_player_id );

			// Whacky Weapons Type; identifies the shot as coming from this plugin and the weapon
			// type it came from.
			bz_setShotMetaData(shot_guid,"wwType",ww_type);
			// The shot is fired by the server, but if it hits anybody, this is who we credit as the
			// killer when they die.
			bz_setShotMetaData(shot_guid,"owner",(uint32_t)from_player_id);

			/*printf("Shot %u \"%s\" from %d (col %d): %.3f %.3f %.3f -> %.3f %.3f %.3f\n",
				shot_guid, type, player_id, (int)color,
				(double)origin[0], (double)origin[1], (double)origin[2],
				(double)vector[0], (double)vector[1], (double)vector[2]
			);*/

			return shot_guid;
		}
		uint32_t fire_shot     ( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos,fvec3 const& vel_dived_by_bzdb_shotspeed                   ) {
			return fire_something( from_player_id, color, "US",ww_type, pos,vel_dived_by_bzdb_shotspeed );
		}
		uint32_t fire_supershot( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos,fvec3 const& vel_dived_by_bzdb_shotspeed                   ) {
			return fire_something( from_player_id, color, "SB",ww_type, pos,vel_dived_by_bzdb_shotspeed );
		}
		uint32_t fire_laser    ( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos,fvec3 const& vel_dived_by_bzdb_shotspeed                   ) {
			return fire_something( from_player_id, color, "L",ww_type, pos,vel_dived_by_bzdb_shotspeed );
		}
		uint32_t fire_laser_to ( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos0,fvec3 const& pos1, float eps=0.01f                        ) {
			/*
			The length of a laser given a `<vector>` is exactly:
				`<vector's length> * _laserAdVel * _shotSpeed * 0.35`
			It is not clear why this is, but it means that if we want to make the laser go exactly
			`<vector length>`, then we need to divide the vector we pass in by:
				`_laserAdVel * _shotSpeed * 0.35`
			*/
			fvec3 vec = pos1 - pos0;
			float length = vec.length();
			float sc = (length+eps) / length; //Scale vector to be a bit longer just to help collisions

			/*
			The length of a laser given a `<vector>` is exactly:
				<laser length> = <vector's length> * _laserAdVel * _shotSpeed * <laser shot lifetime>
			Where:
				<laser shot lifetime> = (_shotRange / _shotSpeed) * _laserAdLife

			If we want to make the laser go exactly `<vector length>`, then by simple algebra we
			need to multiply the vector we pass in by:
				`_shotSpeed / ( _shotSpeed * _shotRange * _laserAdVel * _laserAdLife )`
			*/
			sc *= (float)( bz_getBZDBDouble("_shotSpeed") / (
				bz_getBZDBDouble("_shotSpeed") * bz_getBZDBDouble("_shotRange") * bz_getBZDBDouble("_laserAdVel") * bz_getBZDBDouble("_laserAdLife")
			) );

			//printf("%f %f %f %f\n",bz_getBZDBDouble("_laserAdVel"),bz_getBZDBDouble("_laserAdLife"),bz_getBZDBDouble("_shotRange"),bz_getBZDBDouble("_shotSpeed"));

			return fire_laser( from_player_id, color, ww_type, pos0, sc*vec );
		}
		uint32_t fire_gm       ( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos,fvec3 const& vel_dived_by_bzdb_shotspeed, int at_player_id ) {
			return fire_something( from_player_id, color, "GM",ww_type, pos,vel_dived_by_bzdb_shotspeed, at_player_id );
		}
		uint32_t fire_sw       ( int from_player_id, bz_eTeamType color, char const* ww_type, fvec3 const& pos ) {
			return fire_something( from_player_id, color, "SW",ww_type, pos,fvec3(0.0f,0.0f,0.0f) );
		}
		void     fire_smite( int from_player_id, fvec3 const& pos, char const* ww_type, float radius ) {
			//Makes a "thick" bunch of lasers firing downward

			#define WW_SMITE_NUM_L 8 //Number of lasers to use in beam

			//Points spaced around circle
			fvec2 circle_points[WW_SMITE_NUM_L];
			for (int i=0;i<WW_SMITE_NUM_L;++i) {
				float angle = (float)i * ( (1.0f/(float)WW_SMITE_NUM_L) * 2.0f*PI );
				circle_points[i] = radius * fvec2( std::cos(angle), std::sin(angle) );
			}

			auto draw_laser_between = [&](fvec3 const& relp0,fvec3 const& relp1) -> void {
				fire_laser_to( from_player_id, bz_eTeamType::eRogueTeam, ww_type, pos+relp0,pos+relp1 );
			};

			float height = static_cast<float>(bz_getBZDBDouble("_wwLBstartheight"));
			for (int i=0;i<WW_SMITE_NUM_L;++i) {
				fvec3 relp0  = fvec3( circle_points[ i                                    ], height );
				fvec3 relp1a = fvec3( circle_points[(i+(-1+WW_SMITE_NUM_L))%WW_SMITE_NUM_L],   0.0f );
				fvec3 relp1b = fvec3( circle_points[(i+(+1               ))%WW_SMITE_NUM_L],   0.0f );
				draw_laser_between(relp0,relp1a);
				draw_laser_between(relp0,relp1b);
			}

			#undef WW_SMITE_NUM_L
		}

		void cancel_firing_shot(bz_ShotFiredEventData_V1* data) {
			/*
			The client has added a shot, and has notified the server with a bz_eShotFiredEvent.  We
			need to (1) cancel this event, so that we do not add a shot on a server, nor broadcast
			it to other clients, and (2) we need to tell the client to delete their shots, so that
			the user does not see it.

			Doing (1) is actually fairly easy.  See src/bzfs/bzfs.cxx line 4208 for this
			undocumented feature.  Unfortunately, the user will have already played a sound, but
			there's only so much we can do.
			*/
			data->type = "DELETE";
			data->changed = true;

			/*
			Doing (2) is a bit harder.  We're going to adapt the example of the normal MsgShotEnd
			message (see "src/bzfs/bzfs.cxx" line 4313):
				https://github.com/BZFlag-Dev/bzflag/blob/2.4/src/bzfs/bzfs.cxx#L4313

			The shot ID provided by bz_ShotFiredEventData_V1 was, until recently, bogus.  Please
			ensure your bzfs is up to date if it still looks like clients can shoot.  See also:
				https://gist.github.com/agatha2/3305591e2282b8eb47fbc9742135b171
				https://github.com/BZFlag-Dev/bzflag/pull/260
				https://github.com/BZFlag-Dev/bzflag/commit/bbcf0a888203c40d25dc2bee389bce98ae793eb9
				I.e., this was fixed by release v2.4.22.  Clients with versions less than this will
				be rejected on join (see above).

			This is a hack, and it relies on functionality that isn't in the BZFS API.  This causes
			linker problems on Windows.  (TODO: resolve?)
			*/
			#if 1
			void *buf, *bufStart=getDirectMessageBuffer();
			//Note these handle network byte order too ("nbo")!
			buf = nboPackUByte(bufStart,data->playerID);
			buf = nboPackShort(buf,data->shotID);
			buf = nboPackUShort(buf,1); // no explodey
			directMessage( data->playerID, MsgShotEnd, (char*)buf-(char*)bufStart, bufStart );
			#endif

			//printf("Canceling shot: player %d firing %d!\n",data->playerID,data->shotID);
		}
};

BZ_PLUGIN(WhackyWeapons)
