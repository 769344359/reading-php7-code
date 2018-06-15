[php gc 论文](http://researcher.watson.ibm.com/researcher/files/us-bacon/Bacon01Concurrent.pdf)

> php gc 回收

```Our synchronous algorithm is similar to Lins’ algorithm: when reference counts are decremented,we place potential roots of cyclic garbage into a buffer called Roots
.

```
***
### 1导言
40年前,垃圾回收主要是使用两种方法:
- 引用计数法 和 追踪法(tracing)

由于引用计数法被认为是有缺陷的(:应该指循环引用),所以跟踪(tracing)的和他的变种(标记清除:mark-and-sweep,半空间复制:semispace copying,标记整理:mark-and-compact)


1 Introduction
Forty years ago, two methods of automatic storage reclamation were introduced: reference
counting [7] and tracing [23]. Since that time tracing collectors and their variants
(mark-and-sweep, semispace copying, mark-and-compact) have been much more
widely used due to perceived deficiencies in reference counting.
Changes in the relative costs of memory and processing power, and the adoption
of garbage collected languages in mainstream programming (particularly Java) have
changed the landscape. We believe it is time to take a fresh look at reference counting,
particularly as processor clock speeds increase while RAM becomes plentiful but not
significantly faster. In this environment the locality properties of reference counting
are appealing, while the purported extra processing power required is likely to be less
relevant.
At the same time, Java’s incorporation of garbage collection has thrust the problem
into the mainstream, and large, mission critical systems are being built in Java, stressing
the flexibility and scalability of the underlying garbage collection implementations. As
a result, the supposed advantages of tracing collectors — simplicity and low overhead
— are being eroded as they are being made ever more complex in an attempt to address
the real-world requirements of large and varied programs.
Furthermore, the fundamental assumption behind tracing collectors, namely that it
is acceptable to periodically trace all of the live objects in the heap, will not necessarily
scale to the very large main memories that are becoming increasingly common.

---

首先先确定颜色
```
 * BLACK  (GC_BLACK)   - In use or free.   
 * GREY   (GC_GREY)    - Possible member of cycle.
 * WHITE  (GC_WHITE)   - Member of garbage cycle.
 * PURPLE (GC_PURPLE)  - Possible root of cycle
```
- 黑色 : 正在被引用或者已经被回收了(不需要再关注了)
- 灰色 : 可能会被回收的 (计数可能会为0)
- 白色 : 在gc 中
- 紫色 : root
