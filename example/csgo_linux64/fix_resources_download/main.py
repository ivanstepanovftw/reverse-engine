
# TODO: (WIP) ADD PYTHON TRAINER EXAMPLE!!!!!!!!!!!!!!!!!!!!!
# but you can check ./reverseengine/test/binding/pybind11_search.py

from time import sleep
import reverseengine_swig as RE

target = "csgo_linux64"


def stage_waiting():
    global csgo

    print("1. Waiting for process.")
    print("process:", target)

    csgo = RE.Handle(target)
    while not csgo.is_good():
        csgo.attach(target)
        sleep(1)

    print("2. Found!")
    print("pid:", csgo.pid)
    print("title:", csgo.title)
    print("pid:", csgo.get_path())
    print("~"*70)
    return 0


def stage_updating():
    global csgo
    global engine_client_so

    print("1. Updating regions.")
    while True:
        engine_client_so = csgo.get_region_by_name("engine_client.so")
        print(2, engine_client_so)
        if engine_client_so:
            break
        if not csgo.is_running():
            return 1
        sleep(1)
    print("Regions added: "+csgo.regions.size()+", ignored: "+csgo.regions_ignored.size())
    print("Found region:", engine_client_so)
    return 0


def main():
    global csgo
    global engine_client_so

    while True:
        stage_waiting()
        stage_updating() or print("+++")


if __name__ == '__main__':
    main()
