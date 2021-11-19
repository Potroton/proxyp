#include "events.h"
#include "gt.hpp"
#include "proton/hash.hpp"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "server.h"
#include "utils.h"
#include <thread>
#include <limits.h>
#include <vector>
#include <iostream>
std::vector<std::thread> threads;

bool events::out::variantlist(gameupdatepacket_t* packet) {
    variantlist_t varlist{};
    varlist.serialize_from_mem(utils::get_extended(packet));
    PRINTS("varlist: %s\n", varlist.print().c_str());
    return false;
}

bool events::out::pingreply(gameupdatepacket_t* packet) {
    //since this is a pointer we do not need to copy memory manually again
    packet->m_vec2_x = 1000.f;  //gravity
    packet->m_vec2_y = 260.f;   //move speed
    packet->m_vec_x = 64.f;     //punch range
    packet->m_vec_y = 64.f;     //build range
    packet->m_jump_amount = 0;  //for example unlim jumps set it to high which causes ban
    packet->m_player_flags = 0; //effect flags. good to have as 0 if using mod noclip, or etc.
    return false;
}

bool find_command(std::string chat, std::string name) {
    bool found = chat.find("/" + name) == 0;
    if (found)
        gt::send_log("`6" + chat);
    return found;
}
bool wrench = false;
bool fastdrop = false;
bool fasttrash = false;
bool ghost = false;
std::string userNetID = "";
std::string action = "";



std::string mode = "pull";
bool events::out::generictext(std::string packet) {
    //PRINTS("Generic text: %s\n", packet.c_str());
    auto& world = g_server->m_world;
    rtvar var = rtvar::parse(packet);

    if (!var.valid())
        return false;

    for (int a = 0; a < var.size(); a++) {
        std::string key = var.get(a).m_key;
        std::string value = var.get(a).m_value;
        gt::send_log("`c" + key + "|" + value);
    }

        if (wrench == true) {
            if (packet.find("action|wrench") != -1) {
                g_server->send(false, packet);
                std::string id = packet.substr(packet.find("netid|") + 6, packet.length() - packet.find("netid|") - 1);
                std::string netid = id.substr(0, id.find("|"));
                if (mode.find("pull") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + netid + "|\nnetID|" + netid + "|\nbuttonClicked|pull");
                }
                if (mode.find("kick") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + netid + "|\nnetID|" + netid + "|\nbuttonClicked|kick");
                }
                if (mode.find("ban") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + netid + "|\nnetID|" + netid + "|\nbuttonClicked|wban");
                }
                if (mode.find("friend") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + netid + "|\nnetID|" + netid + "|\nbuttonClicked|addfriendrnbutton");
                }
                if (mode.find("trade") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + netid + "|\nnetID|" + netid + "|\nbuttonClicked|trade");
                }

                if (mode.find("netid") != -1) {
                    gt::send_log("Saved NetID: " + netid);
                    userNetID = netid;
                }
                return true;
            }
        }
    if (var.get(0).m_key == "action" && var.get(0).m_value == "input") {
        if (var.size() < 2)
            return false;
        if (var.get(1).m_values.size() < 2)
            return false;

        if (!world.connected)
            return false;

        auto chat = var.get(1).m_values[1];
        if (find_command(chat, "name ")) { 
            std::string name = "``" + chat.substr(6) + "``";
            variantlist_t va{ "OnNameChanged" };
            va[1] = name;
            g_server->send(true, va, world.local.netid, -1);
            gt::send_log("name set to: " + name);
            return true;
        } else if (find_command(chat, "flag ")) {
            int flag = atoi(chat.substr(6).c_str());
            variantlist_t va{ "OnGuildDataChanged"
            };
            va[1] = 1;
            va[2] = 2;
            va[3] = flag;
            va[4] = 3;
            g_server->send(true, va, world.local.netid, -1);
            gt::send_log("`2Flag set to item id: " + std::to_string(flag));
            return true;
        }
        else if (find_command(chat, "action ")) {
            action = chat.substr(8, chat.length() - 8);
            std::replace(action.begin(), action.end(), ' ', '\n');
            std::replace(action.begin(), action.end(), '.', '|');
            gt::send_log("`2Action set to " + action);
            std::cout << action << std::endl;
            return true;
        }
        else if (find_command(chat, "send")) {

            if (action != "") {
                for (int viam = 0; viam < 60; viam++) {
                    g_server->send(false, action,viam);
                }
                gt::send_log("`2Sent action.");
            }
            else gt::send_log("`4Set an action with /action <action>.");
            return true;

        }
        else if (find_command(chat, "follow")) {
            if (userNetID != "") {
                gt::takip = !gt::takip;
                if (gt::takip) {
                    gt::send_log("`2Follow started.");
                   
                }
                else {
                    gt::send_log("`4Follow stopped.");
                }
            }
            else {
                gt::send_log("None of NetID has been set.");
            }
            return true;
        }

        else if (find_command(chat, "lagspike ")) {
            std::string intensivity = chat.substr(10);
            gt::send_log("`cSending packets...");
            for (int x = 0; x < std::stoi(intensivity); x++) {
                for (int i = 0; i < 6000; i++) {
                    gameupdatepacket_t packet{};
                    packet.m_type = PACKET_TILE_CHANGE_REQUEST;
                    packet.m_int_data = 18;
                    int x = i % 100, y = i / 100;
                    packet.m_state1 = x;
                    packet.m_state2 = y;
                    g_server->send(false, NET_MESSAGE_GAME_PACKET, (uint8_t*)&packet, sizeof(gameupdatepacket_t));
                }
            }
            gt::send_log("`2Sent packets.");
            return true;
        }else if (find_command(chat, "country ")) {
            std::string cy = chat.substr(9);
            gt::flag = cy;
            gt::send_log("`2Your country set to " + cy + ", (Relog to game to change it successfully!)");
            return true;
        }
        else if (find_command(chat, "fd")) {
            fastdrop = !fastdrop;
            if (fastdrop)
                gt::send_log("`2Fast Drop is now enabled.");
            else
                gt::send_log("`4Fast Drop is now disabled.");
            return true;
        }
        else if (find_command(chat, "ft")) {
            fasttrash = !fasttrash;
            if (fasttrash)
                gt::send_log("`2Fast Trash is now enabled.");
            else
                gt::send_log("`4Fast Trash is now disabled.");
            return true;
        }        
        else if (find_command(chat, "wrenchset ")) {
            mode = chat.substr(10);
            gt::send_log("Wrench mode set to " + mode);
            return true;        
        }
        else if (find_command(chat, "wrenchmode")) {
            wrench = !wrench;
            if (wrench)
                gt::send_log("`2Wrench mode is on.");
            else
                gt::send_log("`4Wrench mode is off.");
            return true;
        }
        else if (find_command(chat, "tp")) {
            if (stoi(userNetID) == 0) {
                gt::send_log("`4Please set a netid by /wrenchset netid , then /wrenchmode , after that wrench someone.");
                return true;
            }
            for (auto& player : g_server->m_world.players) {
                if (stoi(userNetID) == player.netid) {
                    gt::send_log("`2Teleporting to " + player.name);
                    variantlist_t varlist{ "OnSetPos" };
                    varlist[1] = player.pos;
                    g_server->m_world.local.pos = player.pos;
                    g_server->send(true, varlist, g_server->m_world.local.netid, -1);
                    break;
                }
            }
            return true;
        } else if (find_command(chat, "warp ")) {
            std::string name = chat.substr(6);
            gt::send_log("`cWarping to " + name);
            g_server->send(false, "action|join_request\nname|" + name + "\ninvitedWorld|0", 3);
            return true;
           } else if (find_command(chat, "pullall")) {
            std::string username = chat.substr(6);
            for (auto& player : g_server->m_world.players) {
                auto name_2 = player.name.substr(2); //remove color
                if (name_2.find(username)) {
                    g_server->send(false, "action|wrench\n|netid|" + std::to_string(player.netid));
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    g_server->send(false, "action|dialog_return\ndialog_name|popup\nnetID|" + std::to_string(player.netid) + "|\nbuttonClicked|pull"); 
                    // You Can |kick |trade |worldban 
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    gt::send_log("`2Pulled");
                  
                }
            }
            return true;
            
        }
           
           else if (find_command(chat, "skin ")) {
            int skin = atoi(chat.substr(6).c_str());
            variantlist_t va{ "OnChangeSkin" };
            va[1] = skin;
            g_server->send(true, va, world.local.netid, -1);
            return true;
        } else if (find_command(chat, "wrench ")) {
            std::string name = chat.substr(6);
            std::string username = ".";
            for (auto& player : g_server->m_world.players) {
                auto name_2 = player.name.substr(2);
                std::transform(name_2.begin(), name_2.end(), name_2.begin(), ::tolower);
                    g_server->send(false, "action|wrench\n|netid|" + std::to_string(player.netid));
            }
            return true;
        } else if (find_command(chat, "proxy")) {
            gt::send_log(
                "/tp [name] (teleports to a player in the world), /lag, /ghost (toggles ghost, you wont move for others when its enabled), /uid "
                "[name] (resolves name to uid), /flag [id] (sets flag to item id), /name [name] (sets name to name)");
            return true;
        } 
        return false;
    }

    
    if (packet.find("game_version|") != -1) {
        rtvar var = rtvar::parse(packet);
        auto mac = utils::generate_mac();
        auto hash_str = mac + "RT";
        auto hash2 = utils::hash((uint8_t*)hash_str.c_str(), hash_str.length());
        var.set("mac", mac);
        var.set("wk", utils::generate_rid());
        var.set("rid", utils::generate_rid());
        var.set("fz", std::to_string(utils::random(INT_MIN, INT_MAX)));
        var.set("zf", std::to_string(utils::random(INT_MIN, INT_MAX)));
        var.set("hash", std::to_string(utils::random(INT_MIN, INT_MAX)));
        var.set("hash2", std::to_string(hash2));
        var.set("meta","ni.com");
        var.set("game_version", gt::version);
        var.set("country", gt::flag);

        packet = var.serialize();
        gt::in_game = false;
        PRINTS("Spoofing login info\n");
        g_server->send(false, packet);
        return true;
    }

    return false;
}

bool events::out::gamemessage(std::string packet) {
    PRINTS("Game message: %s\n", packet.c_str());
    gt::send_log("`cGame message: " + packet);
    if (packet == "action|quit") {
        g_server->quit();
        return true;
    }
    return false;
}

bool events::out::state(gameupdatepacket_t* packet) {
    if (!g_server->m_world.connected)
        return false;

    g_server->m_world.local.pos = vector2_t{ packet->m_vec_x, packet->m_vec_y };
    PRINTS("local pos: %.0f %.0f\n", packet->m_vec_x, packet->m_vec_y);
    
    return false;
}

bool events::in::variantlist(gameupdatepacket_t* packet) {
    variantlist_t varlist{};
    auto extended = utils::get_extended(packet);
    extended += 4; //since it casts to data size not data but too lazy to fix this
    varlist.serialize_from_mem(extended);
    PRINTC("varlist: %s\n", varlist.print().c_str());
    auto func = varlist[0].get_string();

    //probably subject to change, so not including in switch statement.
    if (func.find("OnSuperMainStartAcceptLogon") != -1)
        gt::in_game = true;
        
    switch (hs::hash32(func.c_str())) {
        case fnv32("OnRequestWorldSelectMenu"): {
            auto& world = g_server->m_world;
            world.players.clear();
            world.local = {};
            world.connected = false;
            world.name = "EXIT";
        } break;
        case fnv32("OnSendToServer"): g_server->redirect_server(varlist); return true;

        case fnv32("OnConsoleMessage"): {
            auto content = varlist[1].get_string();
            varlist[1] = "`4[V]`` " + content;
            
            g_server->send(true, varlist);
            
            return true;
        } break;
        case fnv32("OnDialogRequest"): {
            auto content = varlist[1].get_string();

        if (wrench == true) {
            if (content.find("add_button|report_player|") != -1) {
                    return true; // block wrench dialog
            }
        }
        if (content.find("gazette") != -1) { // skip news shit
            
            return true;
        }
        if (fastdrop == true) {
            std::string itemid = content.substr(content.find("embed_data|itemID|") + 18, content.length() - content.find("embed_data|itemID|") - 1);
            std::string count = content.substr(content.find("count||") + 7, content.length() - content.find("count||") - 1);
            if (content.find("embed_data|itemID|") != -1) {
                if (content.find("Drop") != -1) {
                   

                    return true;
                }
            }
        }
        if (fasttrash == true) {
            std::string itemid = content.substr(content.find("embed_data|itemID|") + 18, content.length() - content.find("embed_data|itemID|") - 1);
            std::string count = content.substr(content.find("you have ") + 9, content.length() - content.find("you have ") - 1);
            std::string delimiter = ")";
            std::string token = count.substr(0, count.find(delimiter));
            if (content.find("embed_data|itemID|") != -1) {
                if (content.find("Trash") != -1) {
                    g_server->send(false, "action|dialog_return\ndialog_name|trash_item\nitemID|" + itemid + "|\ncount|" + token);
                    return true;
                }
            }
        }            
            //hide unneeded ui when resolving
            //for the /uid command
            if (gt::resolving_uid2 && (content.find("friend_all|Show offline") != -1 || content.find("Social Portal") != -1) ||
                content.find("Are you sure you wish to stop ignoring") != -1) {
                return true;
            } else if (gt::resolving_uid2 && content.find("Ok, you will now be able to see chat") != -1) {
                gt::resolving_uid2 = false;
                return true;
            } else if (gt::resolving_uid2 && content.find("`4Stop ignoring") != -1) {
                int pos = content.rfind("|`4Stop ignoring");
                auto ignore_substring = content.substr(0, pos);
                auto uid = ignore_substring.substr(ignore_substring.rfind("add_button|") + 11);
                auto uid_int = atoi(uid.c_str());
                if (uid_int == 0) {
                    gt::resolving_uid2 = false;
                    gt::send_log("name resolving seems to have failed.");
                } else {
                    gt::send_log("Target UID: " + uid);
                    g_server->send(false, "action|dialog_return\ndialog_name|friends\nbuttonClicked|" + uid);
                    g_server->send(false, "action|dialog_return\ndialog_name|friends_remove\nfriendID|" + uid + "|\nbuttonClicked|remove");
                }
                return true;
            }
        } break;
        case fnv32("OnRemove"): {
            auto text = varlist.get(1).get_string();
            if (text.find("netID|") == 0) {
                auto netid = atoi(text.substr(6).c_str());

                if (netid == g_server->m_world.local.netid)
                    g_server->m_world.local = {};

                auto& players = g_server->m_world.players;
                for (size_t i = 0; i < players.size(); i++) {
                    auto& player = players[i];
                    if (player.netid == netid) {
                        players.erase(std::remove(players.begin(), players.end(), player), players.end());
                        break;
                    }
                }
                if (gt::takip && stoi(userNetID) == netid) gt::takip = false;
            }
        } break;
        case fnv32("OnSpawn"): {
            std::string meme = varlist.get(1).get_string();
            rtvar var = rtvar::parse(meme);
            auto name = var.find("name");
            auto netid = var.find("netID");

            if (name && netid) {
                player ply{};

                if (var.find("invis")->m_value != "1") {
                    ply.name = name->m_value;
                    ply.country = var.get("country");
                    name->m_values[0] += " `4[" + netid->m_value + "]``";
                    auto pos = var.find("posXY");
                    if (pos && pos->m_values.size() >= 2) {
                        auto x = atoi(pos->m_values[0].c_str());
                        auto y = atoi(pos->m_values[1].c_str());
                        ply.pos = vector2_t{ float(x), float(y) };
                    }
                } else {
                    ply.mod = true;
                    ply.invis = true;
                }
                if (var.get("mstate") == "1" || var.get("smstate") == "1")
                    ply.mod = true;
                ply.userid = var.get_int("userID");
                ply.netid = var.get_int("netID");
                if (meme.find("type|local") != -1) {
                    //set mod state to 1 (allows infinite zooming, this doesnt ban cuz its only the zoom not the actual long punch)
                    var.find("mstate")->m_values[0] = "1";
                    g_server->m_world.local = ply;
                }
                g_server->m_world.players.push_back(ply);
                auto str = var.serialize();
                varlist[1] = str;
                PRINTC("new: %s\n", varlist.print().c_str());
                g_server->send(true, varlist, -1, -1);
                return true;
            }
        } break;
    }
    return false;
}

bool events::in::generictext(std::string packet) {
    PRINTC("Generic text: %s\n", packet.c_str());
    gt::send_log("`cGeneric text: " + packet);
    return false;
}

bool events::in::gamemessage(std::string packet) {
    PRINTC("Game message: %s\n", packet.c_str());
    
    if (gt::resolving_uid2) {
        if (packet.find("PERSON IGNORED") != -1) {
            g_server->send(false, "action|dialog_return\ndialog_name|friends_guilds\nbuttonClicked|showfriend");
            g_server->send(false, "action|dialog_return\ndialog_name|friends\nbuttonClicked|friend_all");
        } else if (packet.find("Nobody is currently online with the name") != -1) {
            gt::resolving_uid2 = false;
            gt::send_log("Target is offline, cant find uid.");
        } else if (packet.find("Clever perhaps") != -1) {
            gt::resolving_uid2 = false;
            gt::send_log("Target is a moderator, can't ignore them.");
        }
    }
    
       
    
    return false;
}

bool events::in::sendmapdata(gameupdatepacket_t* packet) {
    g_server->m_world = {};
    auto extended = utils::get_extended(packet);
    extended += 4;
    auto data = extended + 6;
    auto name_length = *(short*)data;

    char* name = new char[name_length + 1];
    memcpy(name, data + sizeof(short), name_length);
    char none = '\0';
    memcpy(name + name_length, &none, 1);

    g_server->m_world.name = std::string(name);
    g_server->m_world.connected = true;
    delete[] name;
    PRINTC("world name is %s\n", g_server->m_world.name.c_str());
    return false;
}

bool events::in::state(gameupdatepacket_t* packet) {
    if (!g_server->m_world.connected)
        return false;
    if (packet->m_player_flags == -1)
        return false;

    auto& players = g_server->m_world.players;
    for (auto& player : players) {
        if (gt::takip && stoi(userNetID) == player.netid) {
            variantlist_t varlist{ "OnSetPos" };
            varlist[1] = player.pos;
            g_server->m_world.local.pos = player.pos;
            g_server->send(true, varlist, g_server->m_world.local.netid, -1);
        }
        if (player.netid == packet->m_player_flags) {
            player.pos = vector2_t{ packet->m_vec_x, packet->m_vec_y };
            PRINTC("player %s position is %.0f %.0f\n", player.name.c_str(), player.pos.m_x, player.pos.m_y);
            break;
        }
    }
    return false;
}

bool events::in::tracking(std::string packet) {
    PRINTC("Tracking packet: %s\n", packet.c_str());
    return true;
}
