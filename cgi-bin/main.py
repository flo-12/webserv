import argparse
import sys

from scraper import ListingsScraper

#parser = argparse.ArgumentParser()
#parser.add_argument("-release_id")

def main():
    if len(sys.argv) != 2:
        return
    else:
        args = sys.argv[1]
        listings = ListingsScraper().scrape(args)
        for entry in listings:
            print("\n", entry["title"])
            print(entry["href"])
            print(entry["id"])
            print(entry["media_condition"])
            print(entry["sleeve_condition"])
            print(entry["ships_from"])
            print(entry["price"])


if __name__ == "__main__":
    main()
