cd /p/users/bernd/github/pvsim
git init
echo default/ >> .gitignore
echo alt/ >> .gitignore
echo id_rsa >> .gitignore
echo id_rsa.pub >> .gitignore
git add -A
git commit -m "Initial project import"
git branch -M main
git remote add origin git@github.com:kohlstrunk/pvsim.git
git push -u origin main