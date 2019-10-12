### 松糕鞋(sgx)


#### 结构
- xxx.h 定义只在 xxx.c 中使用的宏
- 通用宏在 sgx_core.h 定义

#### 变量解释
- 松糕鞋中除了结构体内部变量名以外所有声明的变量均以 sgx_ 开头
- 结构体 or 特定变量均以 _(说明)_结构体名 结尾，例如：sgx_temp_socket，sgx_tpool