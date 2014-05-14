# CrDiskInfoLib

https://github.com/egtra/crdiskinfolib

CrDiskInfoLibは、HDD情報を取得するライブラリです。
[CrystalDiskInfo](http://crystalmark.info/software/CrystalDiskInfo/)を元に、
C++ライブラリとして使用できるように体裁を整えました。

## ライセンス

Simplified BSD License（2条項BSDライセンス）です。
詳細はCOPYRIGHT.txtまたはCOPYRIGHT-ja.txtを参照下さい。

## 使用法

### コーディング

`CrDiskInfoLib::GetDiskInfo()`を呼び出すと結果が得られます。
検出したHDDの情報が構造体`DiskInfo`の配列(`std::vector<DiskInfo>`)で返されます。

詳しくはSample.cppを見て下さい。
また、構造体`DiskInfo`のメンバーについては、include/DiskInfoLib.hにも一部説明を記述しています。

### ビルド

includeディレクトリをインクルードディレクトリに追加してください。
その上で、DiskInfoLib.hをインクルードしてください。

CrDiskInfoLib.vcxprojまたはCrDiskInfoLib.vcprojをビルドすると静的ライブラリが作られます。
これをリンクしてください。

Visual Studio 2005の場合、別途Windows SDKが必要です。
付属のPlatform SDKではビルドできません。
Windows SDK 7.1でビルドできることを確認しています。
