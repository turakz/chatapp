app.clean:
	rm -rf build
	rm -rf CMakeCache.txt

app.cfg:
	cmake -S. -Bbuild -GNinja

app.cfg.pkgdebug:
	cmake -S. -Bbuild -GNinja --debug-find

app.build:
	cmake --build build
