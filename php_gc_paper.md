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
- 紫色 : root中


---

### 什么时候才可能会变为变为垃圾呢?

1 当引用计数变化的时候可能变为垃圾  
2 引用计数变化只有两种情况: 增加 和 减少,明显增加的时候不会成为被回收对象  
    因为还有人引用他  
3 所以很明显,当计数器减少时候

> 第一步  
当引用计数器减少时  
```
// 当count 减少的实话会触发
ZEND_API void ZEND_FASTCALL gc_possible_root(zend_refcounted *ref)
{
	gc_root_buffer *newRoot;

	if (UNEXPECTED(CG(unclean_shutdown)) || UNEXPECTED(GC_G(gc_active))) {   // 垃圾回收中 或者其他情况   
		return;
	}

	ZEND_ASSERT(GC_TYPE(ref) == IS_ARRAY || GC_TYPE(ref) == IS_OBJECT);
	ZEND_ASSERT(EXPECTED(GC_REF_GET_COLOR(ref) == GC_BLACK));
	ZEND_ASSERT(!GC_ADDRESS(GC_INFO(ref)));

	GC_BENCH_INC(zval_possible_root);

	newRoot = GC_G(unused);
	if (newRoot) {
		GC_G(unused) = newRoot->prev;
	} else if (GC_G(first_unused) != GC_G(last_unused)) {
		newRoot = GC_G(first_unused);
		GC_G(first_unused)++;
	} else {
		if (!GC_G(gc_enabled)) {
			return;
		}
		GC_REFCOUNT(ref)++;
		gc_collect_cycles();
		GC_REFCOUNT(ref)--;
		if (UNEXPECTED(GC_REFCOUNT(ref)) == 0) {
			zval_dtor_func(ref);
			return;
		}
		if (UNEXPECTED(GC_INFO(ref))) {
			return;
		}
		newRoot = GC_G(unused);
		if (!newRoot) {
#if ZEND_GC_DEBUG
			if (!GC_G(gc_full)) {
				fprintf(stderr, "GC: no space to record new root candidate\n");
				GC_G(gc_full) = 1;
			}
#endif
			return;
		}
		GC_G(unused) = newRoot->prev;
	}

	GC_TRACE_SET_COLOR(ref, GC_PURPLE);
	GC_INFO(ref) = (newRoot - GC_G(buf)) | GC_PURPLE;
	newRoot->ref = ref;

	newRoot->next = GC_G(roots).next;
	newRoot->prev = &GC_G(roots);
	GC_G(roots).next->prev = newRoot;
	GC_G(roots).next = newRoot;

	GC_BENCH_INC(zval_buffered);
	GC_BENCH_INC(root_buf_length);
	GC_BENCH_PEAK(root_buf_peak, root_buf_length);
}

```


