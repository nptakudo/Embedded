get: 
	git fetch .
	git pull

commit:
	git add .
	git commit -m "update"
	git push origin main 