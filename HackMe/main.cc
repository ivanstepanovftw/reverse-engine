//
// Created by root on 05.02.18.
//
#include <sys/types.h>
#include <fcntl.h>
#include <linux/input.h>
#include <cstdint>
#include <iostream>
#include <zconf.h>
#include <vector>

using namespace std;

uint64_t r() {
    int64_t r = random();
    return *(uint64_t *)(&r);
}

struct Entry {
    long id;
    float health;
    double x;
    double y;
    double z;
};


class World {
public:
    void Iterate() {
        for(unsigned int i=0; i<30; i++) {
            Entry *e = entries.at(i);
            
            if (e->health <= 0) {
                clog<<e->id<<" is dead. Reanimating now. "<<endl;

                delete e;
                Entry *e_new = new Entry;
                e_new->id = i+50;
                e_new->health = 100;
                e_new->x = -1000.+r()%2000;
                e_new->y = -1000.+r()%2000;
                e_new->z = -1000.+r()%2000;
                entries.at(i) = e_new;
            } else {
                e->x = -10.+r()%20;
                e->y = -10.+r()%20;
                e->z = -10.+r()%20;
            }
        }//for
    }
    
    void CreateWorld(int seed) {
        for(auto e : entries)
            delete e;
        entries.clear();
        for(int i=0; i<30; i++) {
            Entry *e = new Entry;
            e->id = i+50;
            e->health = 100;
            e->x = -1000.+r()%2000;
            e->y = -1000.+r()%2000;
            e->z = -1000.+r()%2000;
            
            entries.push_back(e);
        }
    }
    
    vector<Entry *> *GetEntries() {
        return &entries;
    }
    
private:
    vector<Entry *> entries;
};


class Client {
public:

    Client(World *world) {
        w = world;
        localPlayerId = 5+50;
    }
    
    Entry *GetLocalPlayer() {
        vector<Entry *> *entries = w->GetEntries();
        for(int i=0; i<entries->size(); i++) {
            Entry *e = entries->at(i);
            if (e->id == localPlayerId)
                return e;
        }
        return nullptr;
    }

    float *GetHealth() {
        return &GetLocalPlayer()->health;
    }

    void IncHealth() {
        *GetHealth() += 1+r()%5;
    }

    void DecHealth() {
        *GetHealth() -= 1+r()%5;
    }

    void Suicide() {
        *GetHealth() = 0;
    }

private:
    World *w;
    uint localPlayerId;
};


int
main(int, char **argv) {
    if (getuid() != 0) {
        cerr<<"error: you cannot perform this operation unless you are root."<<endl;
        return 0;
    }
    
    World *w = new World();
    w->CreateWorld(0);
    Client *c = new Client(w);
    
    clog<<"World created, client ready to use!"<<endl;
    
    cout<<"Use numpad for control."<<endl;
    cout<<"1: update world pointer"<<endl;
    cout<<"2: update client pointer"<<endl;
    cout<<"3: new game"<<endl;
    cout<<"6: step main loop"<<endl;
    cout<<"+: increase health"<<endl;
    cout<<"-: decrease health"<<endl;
    cout<<"0: suicide"<<endl;
    cout<<"*: info"<<endl;
    
    bool loop = false;
    bool loopl = false;
    
    int kbd = open("/dev/input/by-path/platform-i8042-serio-0-event-kbd", O_RDONLY);
    if (kbd == -1) {
        cerr << "error while opening keyboard" << endl;
        return 0;
    }
    
    input_event ie{};
    while (read(kbd, &ie, sizeof(ie))) {
        if (ie.type == EV_KEY && ie.value == 1) {
            if (ie.code == KEY_KP1) {
                cout<<"1: update world pointer"<<endl;
                
                delete w;
                w = new World();
                cout << "client isnt working, yea?" << endl;
            }
            else if (ie.code == KEY_KP2) {
                cout<<"2: update client pointer"<<endl;
                delete c;
                c = new Client(w);
            }
            else if (ie.code == KEY_KP3) {
                cout<<"3: new game"<<endl;
                w->CreateWorld(0);
            }
            else if (ie.code == KEY_KP6) {
                cout<<"6: step main loop"<<endl;
                w->Iterate();
            }
            else if (ie.code == KEY_KPPLUS) {
                cout<<"+: increase health"<<endl;
                c->IncHealth();
                cout<<"health: "<<*c->GetHealth()<<endl;
            }
            else if (ie.code == KEY_KPMINUS) {
                cout<<"-: decrease health"<<endl;
                c->DecHealth();
                cout<<"health: "<<*c->GetHealth()<<endl;
            }
            else if (ie.code == KEY_KP0) {
                cout<<"0: suicide"<<endl;
                c->Suicide();
                cout<<"health: "<<*c->GetHealth()<<endl;
                cout<<"dont forget to step"<<endl;
            }
            else if (ie.code == KEY_KPASTERISK) {
                cout<<"*: info"<<endl;
                cout<<"health address at ["<<c->GetHealth()<<"]"<<endl;
                cout<<"Entry address at ["<<c->GetLocalPlayer()<<"]"<<endl;
                cout<<"Entries address at ["<<w->GetEntries()<<"]"<<endl;
                cout<<"Client at ["<<c<<"]"<<endl;
                cout<<"World at ["<<w<<"]"<<endl;
            }

        }
    }
    close(kbd);

}