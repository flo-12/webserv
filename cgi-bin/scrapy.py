#!/usr/bin/python3
import cgi, os
import cgitb
from jinja2 import Environment, FileSystemLoader
import subprocess

"""
def my_var():
	return("eureka")
"""

if __name__ == '__main__':
	cgitb.enable()
	form = cgi.FieldStorage()
	#data = "l"
	"""
	for field in form.keys():
		data += f"{field}: {form[field].value}<br>"
		data += f"l"
	for param in os.environ.keys():
		print("<b>%20s</b>: %s" % (param, os.environ[param]))
	"""
	#data = form["number"].value
	#user_input = form["number"].value
	env = Environment(loader=FileSystemLoader('./cgi-bin'))
	template = env.get_template('scrapy_template.html')
	data = subprocess.check_output(['python3', './cgi-bin/main.py', '10085219'], universal_newlines=True)
	rendered_template = template.render(output=data)
	print(rendered_template)