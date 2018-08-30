# ----------------------------------------------------------------------------
#
#  MIT License
#  
#  Copyright (c) 2016 Abe Takafumi
#  
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included in all
#  copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#  SOFTWARE. *
#
#
# platfoem: linux
# arch    : x86
#
# ----------------------------------------------------------------------------

set(CMAKE_SYSTEM_NAME Linux)

set(TARGET_OS linux)
set(TARGET_ARCH x86)
set(TARGET_PLATFORM linux)
set(TARGET_SUFFIX "${TARGET_OS}.${TARGET_ARCH}")

# ----------------------------------------------------------------------
# gcc静的解析
# ----------------------------------------------------------------------
set(CROSS_FLAGS_C "-Wall")
set(CROSS_FLAGS_CXX "-Wall")

# 関数がaggregate(配列や構造体など?)を返す場合に警告を出す。
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Waggregate-return")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Waggregate-return")

# # 関数呼び出しがマッチしない型にキャストされている場合に警告を出す。
# # C/ObjC
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wbad-function-cast")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wbad-function-cast")

# type qualifier(const,volatileなど)を外すようなポインタのキャストに警告を出す。
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wcast-qual")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wcast-qual")

# # 暗黙型変換のうち、表す値が変わる可能性のあるものに警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wconversion")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wconversion")

# 全てのコンストラクタ・デストラクタがprivateであり、かつfriendもpublic static関数も持たないクラス(=使用できないクラス)に対して警告を出す。
#set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wctor-dtor-privacy")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wctor-dtor-privacy")

# __TIME__,__DATE__,__TIMESTAMP__マクロを使用している場合に警告を出す。
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wdate-time")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wdate-time")

# 仮想関数を持っているのにデストラクタが仮想関数でないクラスに対して、deleteを使っている場合に警告を出す。
#set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wdelete-non-virtual-dtor")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wdelete-non-virtual-dtor")

# コードが長すぎたり複雑すぎたりして、コンパイラが最適化を実行できない場合に警告を出す。
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wdisabled-optimization")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wdisabled-optimization")

# # float型が暗黙にdoubleにキャストされている場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wdouble-promotion")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wdouble-promotion")
# # Scott Meyers の Effective C++ による次の方針に沿わない記述に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Weffc++")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Weffc++")
# # 浮動小数点数を==や!=で比較している場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wfloat-equal")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wfloat-equal")
# # -Wuninitializedが指定されている場合に、初期化されていない変数をそれ自身で初期化している場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Winit-self")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Winit-self")
# # inline指定されている関数を、コンパイラが(関数が長すぎるなどの理由で)インライン展開しなかった場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Winline")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Winline")
# # gotoやswitchで変数宣言を通り過ぎる場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wjump-misses-init")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wjump-misses-init")
# # 論理演算子の間違っているかもしれない使用に対して警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wlogical-op")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wlogical-op")
# # #includeで指定されたディレクトリが見つからない場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wmissing-include-dirs")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wmissing-include-dirs")
# # 複数の文字を含む文字リテラルに対して警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wmultichar")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wmultichar")
# # Cにおいて、規格で定められた文字列長の「最小限の最大長」を超える文字列に対して警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Woverlength-strings")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Woverlength-strings")
# # 派生クラスの関数との名前被りによって、基底クラスのvirtual関数が使えなくなる場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Woverloaded-virtual")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Woverloaded-virtual")
# # 関数型、void型に対するsizeofの適用に対して警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wpointer-arith")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wpointer-arith")
# # コンストラクタのメンバ初期化子と、メンバ変数の宣言の順番が異なる場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wreorder")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wreorder")
# # オーバーロードによってunsigned ~やenumが、signed ~に型変換される場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wsign-promo")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wsign-promo")
# # -fstack-protectorが指定されている場合に、スタック保護がなされなかった関数が存在する場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wstack-protector")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wstack-protector")
# # switch文がdefaultラベルの文を持たない場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wswitch-default")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wswitch-default")
# # switch文の対象になる値がenumで、ラベルがenumのすべての値には対応していない場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wswitch-enum")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wswitch-enum")
# # ループカウンタがオーバーフローする可能性があって、ループを最適化できない場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wunsafe-loop-optimizations")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wunsafe-loop-optimizations")
# # 接尾辞のついていない浮動小数点数リテラルに対して警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wunsuffixed-float-constants")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wunsuffixed-float-constants")
#
# # 変数を、その変数自身の型にキャストしている(無意味である)場合に警告を出す。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wuseless-cast")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wuseless-cast")

# 文字列リテラル(const char*)をchar*へ型変換する場合に警告を出す。
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wwrite-strings")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wwrite-strings")

# # 整数リテラル'0'がヌルポインタを示す定数として使われている場合に警告を出力する。
# set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wzero-as-null-pointer-constant")
# set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wzero-as-null-pointer-constant")

# ----------------------------------------------------------------------
# 警告が出るので、無効化
set(CROSS_FLAGS_C "${CROSS_FLAGS_C} -Wunused-value")
set(CROSS_FLAGS_CXX "${CROSS_FLAGS_CXX} -Wunused-value")

set(CROSS_PREFIX )
set(CROSS_TOOLCHAIN_PATH /usr/)

# コンパイラ設定
set(CMAKE_C_COMPILER ${CROSS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}g++)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CROSS_TOOLCHAIN_PATH}/include)
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CROSS_TOOLCHAIN_PATH}/lib)
set(CMAKE_INSTALL_PREFIX ${CROSS_TOOLCHAIN_PATH}/)

