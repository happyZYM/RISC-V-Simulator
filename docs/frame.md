# 整体框架

cpu应当区分为很多个模块，每个模块应当交由 `CPU` 类进行管理。
在配置好模块后，通过 `add_module` 函数将模块添加到 `CPU` 类中。
这里，可以选择将 unique_ptr 传入，也可以选择将裸指针传入；对于后者，你应该自行确保内存访问安全。

在定义每个模块时，我们认为，模块中需要存储的值由三部分构成：Input, Output 和内部隐藏的数据。

因此，对于每个模块，你需要定义两个或三个**简单结构体**以表示这些数据，然后通过多继承的方法来定义模块。
为了便于进行每个周期的寄存器更新，我们定义了模板类 `Module`, 你只需要继承 `Module` 类并传入你定义的结构体即可。

例如，定义模块 A 如下：

```cpp
struct AInput {
	Wire<1> ready;
	Wire<32> data;
};
struct AOutput {
	Wire<1> valid;
	Wire<32> data;
};
struct AInner {
	Register<32> reg;
};
struct A : Module<AInput, AOutput, AInner> {
	// TODO
};
```

在定义了数据之后，你需要实现模块内部的逻辑。你需要实现 `work` 虚函数，该函数将在每个周期被调用。

```cpp
struct A : Module<AInput, AOutput, AInner> {
	void work() override {
		// TODO
	}
};
```

在实现了模块之后，你可以将模块添加到 `CPU` 类中。

```cpp
CPU cpu;
std::unique_ptr<A> a = std::make_unique<A>();
// TODO: 为 a 连线
cpu.add_module(a);
```

你也可以
```cpp
CPU cpu;
A a; // a's lifetime should be at least as long as cpu's
cpu.add_module(&a);
```

最后，调用 `cpu.run()` 即可开始模拟。

为了能够在结束时退出，你可以通过在 `work` 函数中抛出异常来结束模拟。
当然，你也可以自行修改 work 函数的签名，返回一些信息并在 `run` 函数中判断是否结束模拟。
或者，你可以在 run 函数中检查某个寄存器的值，这些大家可以自行实现。

为了保证正确性，在最终测试中，应当保证模块执行的顺序与运行结果无关。
