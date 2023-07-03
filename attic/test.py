import argparse

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("-c", action="store_true")
    args = ap.parse_args()

    print(args)
