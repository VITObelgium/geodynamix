rm -rf standalone 
mkdir standalone
python build.py --dist --python-path=c:\python312\python.exe --build-dir=gdx-312
python build.py --dist --python-path=c:\python311\python.exe --build-dir=gdx-311
python build.py --dist --python-path=c:\python310\python.exe --build-dir=gdx-310

cp ./build/gdx-312-x64-windows-static-vs2022-dist/geodynamix.cp312-win_amd64.pyd ./standalone
cp ./build/gdx-311-x64-windows-static-vs2022-dist/geodynamix.cp311-win_amd64.pyd ./standalone
cp ./build/gdx-310-x64-windows-static-vs2022-dist/geodynamix.cp310-win_amd64.pyd ./standalone
cp ./build/gdx-310-x64-windows-static-vs2022-dist/proj.db ./standalone