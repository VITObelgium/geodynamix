python build.py --dist --python-path=c:\python310\python.exe --build-dir=gdx-310
python build.py --dist --python-path=c:\python39\python.exe --build-dir=gdx-39
python build.py --dist --python-path=c:\python38\python.exe --build-dir=gdx-38
rm -rf standalone
mkdir standalone
cp ./build/gdx-310-x64-windows-static-vs2022-dist/Release/geodynamix.cp310-win_amd64 ./standalone
cp ./build/gdx-39-x64-windows-static-vs2022-dist/Release/geodynamix.cp39-win_amd64 ./standalone
cp ./build/gdx-38-x64-windows-static-vs2022-dist/Release/geodynamix.cp39-win_amd64 ./standalone