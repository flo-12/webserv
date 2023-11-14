from typing import List

import requests
from bs4 import BeautifulSoup
from bs4.element import ResultSet

class ListingsScraper:
    def __init__(self):
        self.base_url = "https://discogs.com/sell/release/"
        self.url_parameters = "?sort=listed%2Cdesc&limit=25"
    
    def get_listings_soup(self, release_id: int) -> ResultSet:
        headers = {"User-Agent": "friendly"}
        print(f"{self.base_url}{release_id}{self.url_parameters}")
        response = requests.get(f"{self.base_url}{release_id}{self.url_parameters}", headers=headers)
        response.raise_for_status()
        soup = BeautifulSoup(response.text, "html.parser")
        return soup.find_all("tr", {"class": "shortcut_navigable"})

    def parse_listing(self, listing: ResultSet) -> dict:
        item_description_title = listing.find("a", {"class": "item_description_title"})
        title = item_description_title.text
        href = item_description_title.attrs["href"]
        #print(f"Parsing listing with href {href}")
        listing_id = href.split("/")[-1]
        item_condition = listing.find("p", {"class": "item_condition"})
        media_condition = item_condition.find_all("span")[2].text.strip().split(")")[0] + ")"
        try:
            sleeve_condition = item_condition.find_all("span")[8].text.strip()
        except IndexError:
            sleeve_condition = "None"
        seller_info = listing.find("td", {"class": "seller_info"})
        ships_from = seller_info.find_all("li")[2].text.split(":")[-1]
        item_price = listing.find("td", {"class": "item_price"})
        price = item_price.find("span", {"class": "price"}).text
        return {
            "title": title,
            "href": href,
            "id": listing_id,
            "media_condition": media_condition,
            "sleeve_condition": sleeve_condition,
            "ships_from": ships_from,
            "price": price,
        }

    def scrape(self, release_id: int):
        listings = []
        soup_listings = self.get_listings_soup(release_id)
        for listing in soup_listings:
            listings.append(self.parse_listing(listing))
        return listings
