#!/usr/bin/python3
from jinja2 import Environment, FileSystemLoader
import subprocess

"""
def my_var():
	return("eureka")
"""

if __name__ == '__main__':
	env = Environment(loader=FileSystemLoader('./cgi-bin'))
	template = env.get_template('index_template.html')
	rendered_template = template.render()
	print(rendered_template)
