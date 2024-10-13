var U = (t) => {
  throw TypeError(t);
};
var L = (t, e, r) => e.has(t) || U("Cannot " + r);
var w = (t, e, r) => (L(t, e, "read from private field"), r ? r.call(t) : e.get(t)), D = (t, e, r) => e.has(t) ? U("Cannot add the same private member more than once") : e instanceof WeakSet ? e.add(t) : e.set(t, r), k = (t, e, r, s) => (L(t, e, "write to private field"), s ? s.call(t, r) : e.set(t, r), r), j = (t, e, r) => (L(t, e, "access private method"), r);
const PUBLIC_VERSION = "5";
typeof window < "u" && (window.__svelte || (window.__svelte = { v: /* @__PURE__ */ new Set() })).v.add(PUBLIC_VERSION);
const TEMPLATE_USE_IMPORT_NODE = 2, DEV = !1;
var is_array = Array.isArray, array_from = Array.from, define_property = Object.defineProperty, get_descriptor = Object.getOwnPropertyDescriptor;
const noop$1 = () => {
};
function is_promise(t) {
  return typeof (t == null ? void 0 : t.then) == "function";
}
function run_all(t) {
  for (var e = 0; e < t.length; e++)
    t[e]();
}
const DERIVED = 2, EFFECT = 4, RENDER_EFFECT = 8, BLOCK_EFFECT = 16, BRANCH_EFFECT = 32, ROOT_EFFECT = 64, UNOWNED = 128, DISCONNECTED = 256, CLEAN = 512, DIRTY = 1024, MAYBE_DIRTY = 2048, INERT = 4096, DESTROYED = 8192, EFFECT_RAN = 16384, EFFECT_TRANSPARENT = 32768, HEAD_EFFECT = 1 << 18, EFFECT_HAS_DERIVED = 1 << 19;
function equals(t) {
  return t === this.v;
}
function safe_not_equal(t, e) {
  return t != t ? e == e : t !== e || t !== null && typeof t == "object" || typeof t == "function";
}
function safe_equals(t) {
  return !safe_not_equal(t, this.v);
}
function effect_update_depth_exceeded() {
  throw new Error("effect_update_depth_exceeded");
}
function state_unsafe_local_read() {
  throw new Error("state_unsafe_local_read");
}
function state_unsafe_mutation() {
  throw new Error("state_unsafe_mutation");
}
function source(t) {
  return {
    f: 0,
    // TODO ideally we could skip this altogether, but it causes type errors
    v: t,
    reactions: null,
    equals,
    version: 0
  };
}
// @__NO_SIDE_EFFECTS__
function mutable_source(t) {
  var r;
  const e = source(t);
  return e.equals = safe_equals, component_context !== null && component_context.l !== null && ((r = component_context.l).s ?? (r.s = [])).push(e), e;
}
function set(t, e) {
  return active_reaction !== null && is_runes() && active_reaction.f & DERIVED && // If the source was created locally within the current derived, then
  // we allow the mutation.
  (derived_sources === null || !derived_sources.includes(t)) && state_unsafe_mutation(), t.equals(e) || (t.v = e, t.version = increment_version(), mark_reactions(t, DIRTY), is_runes() && active_effect !== null && active_effect.f & CLEAN && !(active_effect.f & BRANCH_EFFECT) && (new_deps !== null && new_deps.includes(t) ? (set_signal_status(active_effect, DIRTY), schedule_effect(active_effect)) : untracked_writes === null ? set_untracked_writes([t]) : untracked_writes.push(t))), e;
}
function mark_reactions(t, e) {
  var r = t.reactions;
  if (r !== null)
    for (var s = is_runes(), o = r.length, u = 0; u < o; u++) {
      var n = r[u], a = n.f;
      a & DIRTY || !s && n === active_effect || (set_signal_status(n, e), a & (CLEAN | UNOWNED) && (a & DERIVED ? mark_reactions(
        /** @type {Derived} */
        n,
        MAYBE_DIRTY
      ) : schedule_effect(
        /** @type {Effect} */
        n
      )));
    }
}
function push_effect(t, e) {
  var r = e.last;
  r === null ? e.last = e.first = t : (r.next = t, t.prev = r, e.last = t);
}
function create_effect(t, e, r, s = !0) {
  var o = (t & ROOT_EFFECT) !== 0, u = active_effect, n = {
    ctx: component_context,
    deps: null,
    nodes_start: null,
    nodes_end: null,
    f: t | DIRTY,
    first: null,
    fn: e,
    last: null,
    next: null,
    parent: o ? null : u,
    prev: null,
    teardown: null,
    transitions: null,
    version: 0
  };
  if (r) {
    var a = is_flushing_effect;
    try {
      set_is_flushing_effect(!0), update_effect(n), n.f |= EFFECT_RAN;
    } catch (l) {
      throw destroy_effect(n), l;
    } finally {
      set_is_flushing_effect(a);
    }
  } else e !== null && schedule_effect(n);
  var f = r && n.deps === null && n.first === null && n.nodes_start === null && n.teardown === null && (n.f & EFFECT_HAS_DERIVED) === 0;
  if (!f && !o && s && (u !== null && push_effect(n, u), active_reaction !== null && active_reaction.f & DERIVED)) {
    var i = (
      /** @type {Derived} */
      active_reaction
    );
    (i.children ?? (i.children = [])).push(n);
  }
  return n;
}
function effect_root(t) {
  const e = create_effect(ROOT_EFFECT, t, !0);
  return () => {
    destroy_effect(e);
  };
}
function effect(t) {
  return create_effect(EFFECT, t, !1);
}
function render_effect(t) {
  return create_effect(RENDER_EFFECT, t, !0);
}
function template_effect(t) {
  return render_effect(t);
}
function block(t, e = 0) {
  return create_effect(RENDER_EFFECT | BLOCK_EFFECT | e, t, !0);
}
function branch(t, e = !0) {
  return create_effect(RENDER_EFFECT | BRANCH_EFFECT, t, !0, e);
}
function execute_effect_teardown(t) {
  var e = t.teardown;
  if (e !== null) {
    const r = active_reaction;
    set_active_reaction(null);
    try {
      e.call(null);
    } finally {
      set_active_reaction(r);
    }
  }
}
function destroy_effect(t, e = !0) {
  var r = !1;
  if ((e || t.f & HEAD_EFFECT) && t.nodes_start !== null) {
    for (var s = t.nodes_start, o = t.nodes_end; s !== null; ) {
      var u = s === o ? null : (
        /** @type {TemplateNode} */
        /* @__PURE__ */ get_next_sibling(s)
      );
      s.remove(), s = u;
    }
    r = !0;
  }
  destroy_effect_children(t, e && !r), remove_reactions(t, 0), set_signal_status(t, DESTROYED);
  var n = t.transitions;
  if (n !== null)
    for (const f of n)
      f.stop();
  execute_effect_teardown(t);
  var a = t.parent;
  a !== null && a.first !== null && unlink_effect(t), t.next = t.prev = t.teardown = t.ctx = t.deps = t.parent = t.fn = t.nodes_start = t.nodes_end = null;
}
function unlink_effect(t) {
  var e = t.parent, r = t.prev, s = t.next;
  r !== null && (r.next = s), s !== null && (s.prev = r), e !== null && (e.first === t && (e.first = s), e.last === t && (e.last = r));
}
function pause_effect(t, e) {
  var r = [];
  pause_children(t, r, !0), run_out_transitions(r, () => {
    destroy_effect(t), e && e();
  });
}
function run_out_transitions(t, e) {
  var r = t.length;
  if (r > 0) {
    var s = () => --r || e();
    for (var o of t)
      o.out(s);
  } else
    e();
}
function pause_children(t, e, r) {
  if (!(t.f & INERT)) {
    if (t.f ^= INERT, t.transitions !== null)
      for (const n of t.transitions)
        (n.is_global || r) && e.push(n);
    for (var s = t.first; s !== null; ) {
      var o = s.next, u = (s.f & EFFECT_TRANSPARENT) !== 0 || (s.f & BRANCH_EFFECT) !== 0;
      pause_children(s, e, u ? r : !1), s = o;
    }
  }
}
function resume_effect(t) {
  resume_children(t, !0);
}
function resume_children(t, e) {
  if (t.f & INERT) {
    t.f ^= INERT, check_dirtiness(t) && update_effect(t);
    for (var r = t.first; r !== null; ) {
      var s = r.next, o = (r.f & EFFECT_TRANSPARENT) !== 0 || (r.f & BRANCH_EFFECT) !== 0;
      resume_children(r, o ? e : !1), r = s;
    }
    if (t.transitions !== null)
      for (const u of t.transitions)
        (u.is_global || e) && u.in();
  }
}
let is_micro_task_queued$1 = !1, current_queued_micro_tasks = [];
function process_micro_tasks() {
  is_micro_task_queued$1 = !1;
  const t = current_queued_micro_tasks.slice();
  current_queued_micro_tasks = [], run_all(t);
}
function queue_micro_task(t) {
  is_micro_task_queued$1 || (is_micro_task_queued$1 = !0, queueMicrotask(process_micro_tasks)), current_queued_micro_tasks.push(t);
}
function flush_tasks() {
  is_micro_task_queued$1 && process_micro_tasks();
}
// @__NO_SIDE_EFFECTS__
function derived(t) {
  let e = DERIVED | DIRTY;
  active_effect === null ? e |= UNOWNED : active_effect.f |= EFFECT_HAS_DERIVED;
  const r = {
    children: null,
    deps: null,
    equals,
    f: e,
    fn: t,
    reactions: null,
    v: (
      /** @type {V} */
      null
    ),
    version: 0,
    parent: active_effect
  };
  if (active_reaction !== null && active_reaction.f & DERIVED) {
    var s = (
      /** @type {Derived} */
      active_reaction
    );
    (s.children ?? (s.children = [])).push(r);
  }
  return r;
}
function destroy_derived_children(t) {
  var e = t.children;
  if (e !== null) {
    t.children = null;
    for (var r = 0; r < e.length; r += 1) {
      var s = e[r];
      s.f & DERIVED ? destroy_derived(
        /** @type {Derived} */
        s
      ) : destroy_effect(
        /** @type {Effect} */
        s
      );
    }
  }
}
function update_derived(t) {
  var e, r = active_effect;
  set_active_effect(t.parent);
  try {
    destroy_derived_children(t), e = update_reaction(t);
  } finally {
    set_active_effect(r);
  }
  var s = (skip_reaction || t.f & UNOWNED) && t.deps !== null ? MAYBE_DIRTY : CLEAN;
  set_signal_status(t, s), t.equals(e) || (t.v = e, t.version = increment_version());
}
function destroy_derived(t) {
  destroy_derived_children(t), remove_reactions(t, 0), set_signal_status(t, DESTROYED), t.children = t.deps = t.reactions = // @ts-expect-error `signal.fn` cannot be `null` while the signal is alive
  t.fn = null;
}
const FLUSH_MICROTASK = 0, FLUSH_SYNC = 1;
let scheduler_mode = FLUSH_MICROTASK, is_micro_task_queued = !1, is_flushing_effect = !1;
function set_is_flushing_effect(t) {
  is_flushing_effect = t;
}
let queued_root_effects = [], flush_count = 0, dev_effect_stack = [], active_reaction = null;
function set_active_reaction(t) {
  active_reaction = t;
}
let active_effect = null;
function set_active_effect(t) {
  active_effect = t;
}
let derived_sources = null, new_deps = null, skipped_deps = 0, untracked_writes = null;
function set_untracked_writes(t) {
  untracked_writes = t;
}
let current_version = 0, skip_reaction = !1, component_context = null;
function set_component_context(t) {
  component_context = t;
}
function increment_version() {
  return ++current_version;
}
function is_runes() {
  return component_context !== null && component_context.l === null;
}
function check_dirtiness(t) {
  var n, a;
  var e = t.f;
  if (e & DIRTY)
    return !0;
  if (e & MAYBE_DIRTY) {
    var r = t.deps, s = (e & UNOWNED) !== 0;
    if (r !== null) {
      var o;
      if (e & DISCONNECTED) {
        for (o = 0; o < r.length; o++)
          ((n = r[o]).reactions ?? (n.reactions = [])).push(t);
        t.f ^= DISCONNECTED;
      }
      for (o = 0; o < r.length; o++) {
        var u = r[o];
        if (check_dirtiness(
          /** @type {Derived} */
          u
        ) && update_derived(
          /** @type {Derived} */
          u
        ), s && active_effect !== null && !skip_reaction && !((a = u == null ? void 0 : u.reactions) != null && a.includes(t)) && (u.reactions ?? (u.reactions = [])).push(t), u.version > t.version)
          return !0;
      }
    }
    s || set_signal_status(t, CLEAN);
  }
  return !1;
}
function handle_error(t, e, r) {
  throw t;
}
function update_reaction(t) {
  var l;
  var e = new_deps, r = skipped_deps, s = untracked_writes, o = active_reaction, u = skip_reaction, n = derived_sources;
  new_deps = /** @type {null | Value[]} */
  null, skipped_deps = 0, untracked_writes = null, active_reaction = t.f & (BRANCH_EFFECT | ROOT_EFFECT) ? null : t, skip_reaction = !is_flushing_effect && (t.f & UNOWNED) !== 0, derived_sources = null;
  try {
    var a = (
      /** @type {Function} */
      (0, t.fn)()
    ), f = t.deps;
    if (new_deps !== null) {
      var i;
      if (remove_reactions(t, skipped_deps), f !== null && skipped_deps > 0)
        for (f.length = skipped_deps + new_deps.length, i = 0; i < new_deps.length; i++)
          f[skipped_deps + i] = new_deps[i];
      else
        t.deps = f = new_deps;
      if (!skip_reaction)
        for (i = skipped_deps; i < f.length; i++)
          ((l = f[i]).reactions ?? (l.reactions = [])).push(t);
    } else f !== null && skipped_deps < f.length && (remove_reactions(t, skipped_deps), f.length = skipped_deps);
    return a;
  } finally {
    new_deps = e, skipped_deps = r, untracked_writes = s, active_reaction = o, skip_reaction = u, derived_sources = n;
  }
}
function remove_reaction(t, e) {
  let r = e.reactions;
  if (r !== null) {
    var s = r.indexOf(t);
    if (s !== -1) {
      var o = r.length - 1;
      o === 0 ? r = e.reactions = null : (r[s] = r[o], r.pop());
    }
  }
  r === null && e.f & DERIVED && // Destroying a child effect while updating a parent effect can cause a dependency to appear
  // to be unused, when in fact it is used by the currently-updating parent. Checking `new_deps`
  // allows us to skip the expensive work of disconnecting and immediately reconnecting it
  (new_deps === null || !new_deps.includes(e)) && (set_signal_status(e, MAYBE_DIRTY), e.f & (UNOWNED | DISCONNECTED) || (e.f ^= DISCONNECTED), remove_reactions(
    /** @type {Derived} **/
    e,
    0
  ));
}
function remove_reactions(t, e) {
  var r = t.deps;
  if (r !== null)
    for (var s = e; s < r.length; s++)
      remove_reaction(t, r[s]);
}
function destroy_effect_children(t, e = !1) {
  var r = t.first;
  for (t.first = t.last = null; r !== null; ) {
    var s = r.next;
    destroy_effect(r, e), r = s;
  }
}
function update_effect(t) {
  var e = t.f;
  if (!(e & DESTROYED)) {
    set_signal_status(t, CLEAN);
    var r = active_effect, s = component_context;
    active_effect = t, component_context = t.ctx;
    try {
      e & BLOCK_EFFECT || destroy_effect_children(t), execute_effect_teardown(t);
      var o = update_reaction(t);
      t.teardown = typeof o == "function" ? o : null, t.version = current_version;
    } catch (u) {
      handle_error(
        /** @type {Error} */
        u
      );
    } finally {
      active_effect = r, component_context = s;
    }
  }
}
function infinite_loop_guard() {
  flush_count > 1e3 && (flush_count = 0, effect_update_depth_exceeded()), flush_count++;
}
function flush_queued_root_effects(t) {
  var e = t.length;
  if (e !== 0) {
    infinite_loop_guard();
    var r = is_flushing_effect;
    is_flushing_effect = !0;
    try {
      for (var s = 0; s < e; s++) {
        var o = t[s];
        o.f & CLEAN || (o.f ^= CLEAN);
        var u = [];
        process_effects(o, u), flush_queued_effects(u);
      }
    } finally {
      is_flushing_effect = r;
    }
  }
}
function flush_queued_effects(t) {
  var e = t.length;
  if (e !== 0)
    for (var r = 0; r < e; r++) {
      var s = t[r];
      !(s.f & (DESTROYED | INERT)) && check_dirtiness(s) && (update_effect(s), s.deps === null && s.first === null && s.nodes_start === null && (s.teardown === null ? unlink_effect(s) : s.fn = null));
    }
}
function process_deferred() {
  if (is_micro_task_queued = !1, flush_count > 1001)
    return;
  const t = queued_root_effects;
  queued_root_effects = [], flush_queued_root_effects(t), is_micro_task_queued || (flush_count = 0);
}
function schedule_effect(t) {
  scheduler_mode === FLUSH_MICROTASK && (is_micro_task_queued || (is_micro_task_queued = !0, queueMicrotask(process_deferred)));
  for (var e = t; e.parent !== null; ) {
    e = e.parent;
    var r = e.f;
    if (r & (ROOT_EFFECT | BRANCH_EFFECT)) {
      if (!(r & CLEAN)) return;
      e.f ^= CLEAN;
    }
  }
  queued_root_effects.push(e);
}
function process_effects(t, e) {
  var r = t.first, s = [];
  e: for (; r !== null; ) {
    var o = r.f, u = (o & BRANCH_EFFECT) !== 0, n = u && (o & CLEAN) !== 0;
    if (!n && !(o & INERT))
      if (o & RENDER_EFFECT) {
        u ? r.f ^= CLEAN : check_dirtiness(r) && update_effect(r);
        var a = r.first;
        if (a !== null) {
          r = a;
          continue;
        }
      } else o & EFFECT && s.push(r);
    var f = r.next;
    if (f === null) {
      let c = r.parent;
      for (; c !== null; ) {
        if (t === c)
          break e;
        var i = c.next;
        if (i !== null) {
          r = i;
          continue e;
        }
        c = c.parent;
      }
    }
    r = f;
  }
  for (var l = 0; l < s.length; l++)
    a = s[l], e.push(a), process_effects(a, e);
}
function flush_sync(t) {
  var e = scheduler_mode, r = queued_root_effects;
  try {
    infinite_loop_guard();
    const o = [];
    scheduler_mode = FLUSH_SYNC, queued_root_effects = o, is_micro_task_queued = !1, flush_queued_root_effects(r);
    var s = t == null ? void 0 : t();
    return flush_tasks(), (queued_root_effects.length > 0 || o.length > 0) && flush_sync(), flush_count = 0, s;
  } finally {
    scheduler_mode = e, queued_root_effects = r;
  }
}
function get$1(t) {
  var e = t.f;
  if (e & DESTROYED)
    return t.v;
  if (active_reaction !== null) {
    derived_sources !== null && derived_sources.includes(t) && state_unsafe_local_read();
    var r = active_reaction.deps;
    new_deps === null && r !== null && r[skipped_deps] === t ? skipped_deps++ : new_deps === null ? new_deps = [t] : new_deps.push(t), untracked_writes !== null && active_effect !== null && active_effect.f & CLEAN && !(active_effect.f & BRANCH_EFFECT) && untracked_writes.includes(t) && (set_signal_status(active_effect, DIRTY), schedule_effect(active_effect));
  }
  if (e & DERIVED) {
    var s = (
      /** @type {Derived} */
      t
    );
    check_dirtiness(s) && update_derived(s);
  }
  return t.v;
}
function untrack(t) {
  const e = active_reaction;
  try {
    return active_reaction = null, t();
  } finally {
    active_reaction = e;
  }
}
const STATUS_MASK = ~(DIRTY | MAYBE_DIRTY | CLEAN);
function set_signal_status(t, e) {
  t.f = t.f & STATUS_MASK | e;
}
function push(t, e = !1, r) {
  component_context = {
    p: component_context,
    c: null,
    e: null,
    m: !1,
    s: t,
    x: null,
    l: null
  }, e || (component_context.l = {
    s: null,
    u: null,
    r1: [],
    r2: source(!1)
  });
}
function pop(t) {
  const e = component_context;
  if (e !== null) {
    const n = e.e;
    if (n !== null) {
      var r = active_effect, s = active_reaction;
      e.e = null;
      try {
        for (var o = 0; o < n.length; o++) {
          var u = n[o];
          set_active_effect(u.effect), set_active_reaction(u.reaction), effect(u.fn);
        }
      } finally {
        set_active_effect(r), set_active_reaction(s);
      }
    }
    component_context = e.p, e.m = !0;
  }
  return (
    /** @type {T} */
    {}
  );
}
var $window, first_child_getter, next_sibling_getter;
function init_operations() {
  if ($window === void 0) {
    $window = window;
    var t = Element.prototype, e = Node.prototype;
    first_child_getter = get_descriptor(e, "firstChild").get, next_sibling_getter = get_descriptor(e, "nextSibling").get, t.__click = void 0, t.__className = "", t.__attributes = null, t.__e = void 0, Text.prototype.__t = void 0;
  }
}
function create_text(t = "") {
  return document.createTextNode(t);
}
// @__NO_SIDE_EFFECTS__
function get_first_child(t) {
  return first_child_getter.call(t);
}
// @__NO_SIDE_EFFECTS__
function get_next_sibling(t) {
  return next_sibling_getter.call(t);
}
function child(t) {
  return /* @__PURE__ */ get_first_child(t);
}
function first_child(t, e) {
  {
    var r = (
      /** @type {DocumentFragment} */
      /* @__PURE__ */ get_first_child(
        /** @type {Node} */
        t
      )
    );
    return r instanceof Comment && r.data === "" ? /* @__PURE__ */ get_next_sibling(r) : r;
  }
}
const all_registered_events = /* @__PURE__ */ new Set(), root_event_handles = /* @__PURE__ */ new Set();
function handle_event_propagation(t) {
  var g;
  var e = this, r = (
    /** @type {Node} */
    e.ownerDocument
  ), s = t.type, o = ((g = t.composedPath) == null ? void 0 : g.call(t)) || [], u = (
    /** @type {null | Element} */
    o[0] || t.target
  ), n = 0, a = t.__root;
  if (a) {
    var f = o.indexOf(a);
    if (f !== -1 && (e === document || e === /** @type {any} */
    window)) {
      t.__root = e;
      return;
    }
    var i = o.indexOf(e);
    if (i === -1)
      return;
    f <= i && (n = f);
  }
  if (u = /** @type {Element} */
  o[n] || t.target, u !== e) {
    define_property(t, "currentTarget", {
      configurable: !0,
      get() {
        return u || r;
      }
    });
    try {
      for (var l, c = []; u !== null; ) {
        var h = u.assignedSlot || u.parentNode || /** @type {any} */
        u.host || null;
        try {
          var d = u["__" + s];
          if (d !== void 0 && !/** @type {any} */
          u.disabled)
            if (is_array(d)) {
              var [p, ...v] = d;
              p.apply(u, [t, ...v]);
            } else
              d.call(u, t);
        } catch (m) {
          l ? c.push(m) : l = m;
        }
        if (t.cancelBubble || h === e || h === null)
          break;
        u = h;
      }
      if (l) {
        for (let m of c)
          queueMicrotask(() => {
            throw m;
          });
        throw l;
      }
    } finally {
      t.__root = e, delete t.currentTarget;
    }
  }
}
function create_fragment_from_html(t) {
  var e = document.createElement("template");
  return e.innerHTML = t, e.content;
}
function assign_nodes(t, e) {
  var r = (
    /** @type {Effect} */
    active_effect
  );
  r.nodes_start === null && (r.nodes_start = t, r.nodes_end = e);
}
// @__NO_SIDE_EFFECTS__
function template(t, e) {
  var r = (e & TEMPLATE_USE_IMPORT_NODE) !== 0, s, o = !t.startsWith("<!>");
  return () => {
    s === void 0 && (s = create_fragment_from_html(o ? t : "<!>" + t), s = /** @type {Node} */
    /* @__PURE__ */ get_first_child(s));
    var u = (
      /** @type {TemplateNode} */
      r ? document.importNode(s, !0) : s.cloneNode(!0)
    );
    return assign_nodes(u, u), u;
  };
}
function comment() {
  var t = document.createDocumentFragment(), e = document.createComment(""), r = create_text();
  return t.append(e, r), assign_nodes(e, r), t;
}
function append(t, e) {
  t !== null && t.before(
    /** @type {Node} */
    e
  );
}
const PASSIVE_EVENTS = ["touchstart", "touchmove"];
function is_passive_event(t) {
  return PASSIVE_EVENTS.includes(t);
}
function set_text(t, e) {
  e !== (t.__t ?? (t.__t = t.nodeValue)) && (t.__t = e, t.nodeValue = e == null ? "" : e + "");
}
function mount(t, e) {
  return _mount(t, e);
}
const document_listeners = /* @__PURE__ */ new Map();
function _mount(t, { target: e, anchor: r, props: s = {}, events: o, context: u, intro: n = !0 }) {
  init_operations();
  var a = /* @__PURE__ */ new Set(), f = (c) => {
    for (var h = 0; h < c.length; h++) {
      var d = c[h];
      if (!a.has(d)) {
        a.add(d);
        var p = is_passive_event(d);
        e.addEventListener(d, handle_event_propagation, { passive: p });
        var v = document_listeners.get(d);
        v === void 0 ? (document.addEventListener(d, handle_event_propagation, { passive: p }), document_listeners.set(d, 1)) : document_listeners.set(d, v + 1);
      }
    }
  };
  f(array_from(all_registered_events)), root_event_handles.add(f);
  var i = void 0, l = effect_root(() => {
    var c = r ?? e.appendChild(create_text());
    return branch(() => {
      if (u) {
        push({});
        var h = (
          /** @type {ComponentContext} */
          component_context
        );
        h.c = u;
      }
      o && (s.$$events = o), i = t(c, s) || {}, u && pop();
    }), () => {
      var p;
      for (var h of a) {
        e.removeEventListener(h, handle_event_propagation);
        var d = (
          /** @type {number} */
          document_listeners.get(h)
        );
        --d === 0 ? (document.removeEventListener(h, handle_event_propagation), document_listeners.delete(h)) : document_listeners.set(h, d);
      }
      root_event_handles.delete(f), mounted_components.delete(i), c !== r && ((p = c.parentNode) == null || p.removeChild(c));
    };
  });
  return mounted_components.set(i, l), i;
}
let mounted_components = /* @__PURE__ */ new WeakMap();
function unmount(t) {
  const e = mounted_components.get(t);
  e && e();
}
const PENDING = 0, THEN = 1, CATCH = 2;
function await_block(t, e, r, s, o) {
  var u = t, n = is_runes(), a = component_context, f, i, l, c, h = (n ? source : mutable_source)(
    /** @type {V} */
    void 0
  ), d = (n ? source : mutable_source)(void 0), p = !1;
  function v(m, _) {
    p = !0, _ && (set_active_effect(g), set_active_reaction(g), set_component_context(a)), m === PENDING && r && (i ? resume_effect(i) : i = branch(() => r(u))), m === THEN && s && (l ? resume_effect(l) : l = branch(() => s(u, h))), m === CATCH && o && (c ? resume_effect(c) : c = branch(() => o(u, d))), m !== PENDING && i && pause_effect(i, () => i = null), m !== THEN && l && pause_effect(l, () => l = null), m !== CATCH && c && pause_effect(c, () => c = null), _ && (set_component_context(null), set_active_reaction(null), set_active_effect(null), flush_sync());
  }
  var g = block(() => {
    if (f !== (f = e())) {
      if (is_promise(f)) {
        var m = f;
        p = !1, m.then(
          (_) => {
            m === f && (set(h, _), v(THEN, !0));
          },
          (_) => {
            m === f && (set(d, _), v(CATCH, !0));
          }
        ), queue_micro_task(() => {
          p || v(PENDING, !0);
        });
      } else
        set(h, f), v(THEN, !1);
      return () => f = null;
    }
  });
}
function action(t, e, r) {
  effect(() => {
    var s = untrack(() => e(t, r == null ? void 0 : r()) || {});
    if (s != null && s.destroy)
      return () => (
        /** @type {Function} */
        s.destroy()
      );
  });
}
function subscribe_to_store(t, e, r) {
  if (t == null)
    return e(void 0), noop$1;
  const s = t.subscribe(
    e,
    // @ts-expect-error
    r
  );
  return s.unsubscribe ? () => s.unsubscribe() : s;
}
var I, W, S, B, C, M, $, T, V;
let Service$1 = (V = class {
  constructor({ route: e, host: r, port: s }) {
    D(this, I);
    D(this, S);
    // 0 idle, 1 has connected, 2 keep reconnecting, 3 stopped
    D(this, B);
    D(this, C);
    D(this, M);
    D(this, $);
    D(this, T);
    k(this, $, `${r ?? "localhost"}${s != null ? `:${s}` : ""}${e ?? "/"}`), k(this, S, 0), k(this, B, null), k(this, C, null), k(this, M, null), k(this, T, null);
  }
  on_message(e) {
    k(this, M, e);
  }
  on_connect(e) {
    k(this, B, e);
  }
  on_disconnect(e) {
    k(this, C, e);
  }
  // non-blocking (run is blocking in c++)
  // calling it again should not do anything
  start() {
    w(this, T) == null && w(this, S) !== 1 && w(this, S) !== 3 && (k(this, T, new WebSocket(`ws://${w(this, $)}`)), w(this, T).binaryType = "arraybuffer", w(this, T).onopen = () => {
      k(this, S, 1), w(this, B) && w(this, B).call(this, w(this, $));
    }, w(this, T).onclose = () => {
      w(this, S) === 1 && (w(this, C) && w(this, C).call(this, w(this, $)), k(this, S, 2), j(this, I, W).call(this));
    }, w(this, T).onerror = () => {
      w(this, S) !== 1 && w(this, S) !== 3 && (k(this, S, 2), j(this, I, W).call(this));
    }, w(this, T).onmessage = (e) => {
      w(this, M) && w(this, M).call(this, w(this, $), new Uint8Array(e.data));
    });
  }
  stop() {
    k(this, S, 3), w(this, T) != null && (w(this, T).close(), k(this, T, null));
  }
  restart() {
    k(this, S, 0);
  }
  publish(e) {
    w(this, T) != null && w(this, S) == 1 && w(this, T).send(e);
  }
  send(e, r) {
    e === w(this, $) && this.publish(r);
  }
}, I = new WeakSet(), W = function() {
  w(this, T) != null && (w(this, T).close(), k(this, T, null)), setTimeout(() => this.start(), 1e3);
}, S = new WeakMap(), B = new WeakMap(), C = new WeakMap(), M = new WeakMap(), $ = new WeakMap(), T = new WeakMap(), V);
const header = (t) => {
  const e = new Uint8Array(4);
  return new DataView(e.buffer).setUint32(0, t, !1), e;
}, concat = (t) => {
  const e = new Uint8Array(t.reduce((s, o) => s + o.length, 0));
  let r = 0;
  for (const s of t)
    e.set(s, r), r += s.length;
  return e;
}, test_connection = async (t) => new Promise((e, r) => {
  const s = new Service$1(t), o = setTimeout(() => {
    s.stop(), r("connection failed... stopping");
  }, 2500);
  s.on_connect(() => {
    console.log("connection tested... proceeding"), clearTimeout(o), s.stop(), e();
  }), s.start();
}), service_fetch = async (t, e, r) => new Promise((s, o) => {
  const u = new Service$1(t), n = setTimeout(() => {
    o("timed out, cancelled");
  }, 2500);
  u.on_connect(() => u.publish(e)), u.on_message((a, f) => {
    const i = new DataView(f.buffer).getUint32(0, !1), l = f.slice(4), c = r(i, l);
    c != null && (s(c), clearTimeout(n), u.stop());
  }), u.start();
}), service_publish = (t, e) => {
  const r = new Service$1(t), s = setTimeout(() => {
    r.stop();
  }, 2500);
  r.on_connect(() => {
    clearTimeout(s), r.publish(e), r.stop();
  }), r.start();
};
function WorkerWrapper(t) {
  return new Worker(
    "/assets/bundler.js",
    {
      name: t == null ? void 0 : t.name
    }
  );
}
var commonjsGlobal = typeof globalThis < "u" ? globalThis : typeof window < "u" ? window : typeof global < "u" ? global : typeof self < "u" ? self : {};
function getDefaultExportFromCjs(t) {
  return t && t.__esModule && Object.prototype.hasOwnProperty.call(t, "default") ? t.default : t;
}
var indexLight = { exports: {} }, indexMinimal = {}, minimal = {}, aspromise = asPromise$1;
function asPromise$1(t, e) {
  for (var r = new Array(arguments.length - 1), s = 0, o = 2, u = !0; o < arguments.length; )
    r[s++] = arguments[o++];
  return new Promise(function(a, f) {
    r[s] = function(l) {
      if (u)
        if (u = !1, l)
          f(l);
        else {
          for (var c = new Array(arguments.length - 1), h = 0; h < c.length; )
            c[h++] = arguments[h];
          a.apply(null, c);
        }
    };
    try {
      t.apply(e || null, r);
    } catch (i) {
      u && (u = !1, f(i));
    }
  });
}
var base64$1 = {};
(function(t) {
  var e = t;
  e.length = function(a) {
    var f = a.length;
    if (!f)
      return 0;
    for (var i = 0; --f % 4 > 1 && a.charAt(f) === "="; )
      ++i;
    return Math.ceil(a.length * 3) / 4 - i;
  };
  for (var r = new Array(64), s = new Array(123), o = 0; o < 64; )
    s[r[o] = o < 26 ? o + 65 : o < 52 ? o + 71 : o < 62 ? o - 4 : o - 59 | 43] = o++;
  e.encode = function(a, f, i) {
    for (var l = null, c = [], h = 0, d = 0, p; f < i; ) {
      var v = a[f++];
      switch (d) {
        case 0:
          c[h++] = r[v >> 2], p = (v & 3) << 4, d = 1;
          break;
        case 1:
          c[h++] = r[p | v >> 4], p = (v & 15) << 2, d = 2;
          break;
        case 2:
          c[h++] = r[p | v >> 6], c[h++] = r[v & 63], d = 0;
          break;
      }
      h > 8191 && ((l || (l = [])).push(String.fromCharCode.apply(String, c)), h = 0);
    }
    return d && (c[h++] = r[p], c[h++] = 61, d === 1 && (c[h++] = 61)), l ? (h && l.push(String.fromCharCode.apply(String, c.slice(0, h))), l.join("")) : String.fromCharCode.apply(String, c.slice(0, h));
  };
  var u = "invalid encoding";
  e.decode = function(a, f, i) {
    for (var l = i, c = 0, h, d = 0; d < a.length; ) {
      var p = a.charCodeAt(d++);
      if (p === 61 && c > 1)
        break;
      if ((p = s[p]) === void 0)
        throw Error(u);
      switch (c) {
        case 0:
          h = p, c = 1;
          break;
        case 1:
          f[i++] = h << 2 | (p & 48) >> 4, h = p, c = 2;
          break;
        case 2:
          f[i++] = (h & 15) << 4 | (p & 60) >> 2, h = p, c = 3;
          break;
        case 3:
          f[i++] = (h & 3) << 6 | p, c = 0;
          break;
      }
    }
    if (c === 1)
      throw Error(u);
    return i - l;
  }, e.test = function(a) {
    return /^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$/.test(a);
  };
})(base64$1);
var eventemitter = EventEmitter;
function EventEmitter() {
  this._listeners = {};
}
EventEmitter.prototype.on = function(e, r, s) {
  return (this._listeners[e] || (this._listeners[e] = [])).push({
    fn: r,
    ctx: s || this
  }), this;
};
EventEmitter.prototype.off = function(e, r) {
  if (e === void 0)
    this._listeners = {};
  else if (r === void 0)
    this._listeners[e] = [];
  else
    for (var s = this._listeners[e], o = 0; o < s.length; )
      s[o].fn === r ? s.splice(o, 1) : ++o;
  return this;
};
EventEmitter.prototype.emit = function(e) {
  var r = this._listeners[e];
  if (r) {
    for (var s = [], o = 1; o < arguments.length; )
      s.push(arguments[o++]);
    for (o = 0; o < r.length; )
      r[o].fn.apply(r[o++].ctx, s);
  }
  return this;
};
var float = factory(factory);
function factory(t) {
  return typeof Float32Array < "u" ? function() {
    var e = new Float32Array([-0]), r = new Uint8Array(e.buffer), s = r[3] === 128;
    function o(f, i, l) {
      e[0] = f, i[l] = r[0], i[l + 1] = r[1], i[l + 2] = r[2], i[l + 3] = r[3];
    }
    function u(f, i, l) {
      e[0] = f, i[l] = r[3], i[l + 1] = r[2], i[l + 2] = r[1], i[l + 3] = r[0];
    }
    t.writeFloatLE = s ? o : u, t.writeFloatBE = s ? u : o;
    function n(f, i) {
      return r[0] = f[i], r[1] = f[i + 1], r[2] = f[i + 2], r[3] = f[i + 3], e[0];
    }
    function a(f, i) {
      return r[3] = f[i], r[2] = f[i + 1], r[1] = f[i + 2], r[0] = f[i + 3], e[0];
    }
    t.readFloatLE = s ? n : a, t.readFloatBE = s ? a : n;
  }() : function() {
    function e(s, o, u, n) {
      var a = o < 0 ? 1 : 0;
      if (a && (o = -o), o === 0)
        s(1 / o > 0 ? (
          /* positive */
          0
        ) : (
          /* negative 0 */
          2147483648
        ), u, n);
      else if (isNaN(o))
        s(2143289344, u, n);
      else if (o > 34028234663852886e22)
        s((a << 31 | 2139095040) >>> 0, u, n);
      else if (o < 11754943508222875e-54)
        s((a << 31 | Math.round(o / 1401298464324817e-60)) >>> 0, u, n);
      else {
        var f = Math.floor(Math.log(o) / Math.LN2), i = Math.round(o * Math.pow(2, -f) * 8388608) & 8388607;
        s((a << 31 | f + 127 << 23 | i) >>> 0, u, n);
      }
    }
    t.writeFloatLE = e.bind(null, writeUintLE), t.writeFloatBE = e.bind(null, writeUintBE);
    function r(s, o, u) {
      var n = s(o, u), a = (n >> 31) * 2 + 1, f = n >>> 23 & 255, i = n & 8388607;
      return f === 255 ? i ? NaN : a * (1 / 0) : f === 0 ? a * 1401298464324817e-60 * i : a * Math.pow(2, f - 150) * (i + 8388608);
    }
    t.readFloatLE = r.bind(null, readUintLE), t.readFloatBE = r.bind(null, readUintBE);
  }(), typeof Float64Array < "u" ? function() {
    var e = new Float64Array([-0]), r = new Uint8Array(e.buffer), s = r[7] === 128;
    function o(f, i, l) {
      e[0] = f, i[l] = r[0], i[l + 1] = r[1], i[l + 2] = r[2], i[l + 3] = r[3], i[l + 4] = r[4], i[l + 5] = r[5], i[l + 6] = r[6], i[l + 7] = r[7];
    }
    function u(f, i, l) {
      e[0] = f, i[l] = r[7], i[l + 1] = r[6], i[l + 2] = r[5], i[l + 3] = r[4], i[l + 4] = r[3], i[l + 5] = r[2], i[l + 6] = r[1], i[l + 7] = r[0];
    }
    t.writeDoubleLE = s ? o : u, t.writeDoubleBE = s ? u : o;
    function n(f, i) {
      return r[0] = f[i], r[1] = f[i + 1], r[2] = f[i + 2], r[3] = f[i + 3], r[4] = f[i + 4], r[5] = f[i + 5], r[6] = f[i + 6], r[7] = f[i + 7], e[0];
    }
    function a(f, i) {
      return r[7] = f[i], r[6] = f[i + 1], r[5] = f[i + 2], r[4] = f[i + 3], r[3] = f[i + 4], r[2] = f[i + 5], r[1] = f[i + 6], r[0] = f[i + 7], e[0];
    }
    t.readDoubleLE = s ? n : a, t.readDoubleBE = s ? a : n;
  }() : function() {
    function e(s, o, u, n, a, f) {
      var i = n < 0 ? 1 : 0;
      if (i && (n = -n), n === 0)
        s(0, a, f + o), s(1 / n > 0 ? (
          /* positive */
          0
        ) : (
          /* negative 0 */
          2147483648
        ), a, f + u);
      else if (isNaN(n))
        s(0, a, f + o), s(2146959360, a, f + u);
      else if (n > 17976931348623157e292)
        s(0, a, f + o), s((i << 31 | 2146435072) >>> 0, a, f + u);
      else {
        var l;
        if (n < 22250738585072014e-324)
          l = n / 5e-324, s(l >>> 0, a, f + o), s((i << 31 | l / 4294967296) >>> 0, a, f + u);
        else {
          var c = Math.floor(Math.log(n) / Math.LN2);
          c === 1024 && (c = 1023), l = n * Math.pow(2, -c), s(l * 4503599627370496 >>> 0, a, f + o), s((i << 31 | c + 1023 << 20 | l * 1048576 & 1048575) >>> 0, a, f + u);
        }
      }
    }
    t.writeDoubleLE = e.bind(null, writeUintLE, 0, 4), t.writeDoubleBE = e.bind(null, writeUintBE, 4, 0);
    function r(s, o, u, n, a) {
      var f = s(n, a + o), i = s(n, a + u), l = (i >> 31) * 2 + 1, c = i >>> 20 & 2047, h = 4294967296 * (i & 1048575) + f;
      return c === 2047 ? h ? NaN : l * (1 / 0) : c === 0 ? l * 5e-324 * h : l * Math.pow(2, c - 1075) * (h + 4503599627370496);
    }
    t.readDoubleLE = r.bind(null, readUintLE, 0, 4), t.readDoubleBE = r.bind(null, readUintBE, 4, 0);
  }(), t;
}
function writeUintLE(t, e, r) {
  e[r] = t & 255, e[r + 1] = t >>> 8 & 255, e[r + 2] = t >>> 16 & 255, e[r + 3] = t >>> 24;
}
function writeUintBE(t, e, r) {
  e[r] = t >>> 24, e[r + 1] = t >>> 16 & 255, e[r + 2] = t >>> 8 & 255, e[r + 3] = t & 255;
}
function readUintLE(t, e) {
  return (t[e] | t[e + 1] << 8 | t[e + 2] << 16 | t[e + 3] << 24) >>> 0;
}
function readUintBE(t, e) {
  return (t[e] << 24 | t[e + 1] << 16 | t[e + 2] << 8 | t[e + 3]) >>> 0;
}
var inquire_1 = inquire$1;
function inquire$1(moduleName) {
  try {
    var mod = eval("quire".replace(/^/, "re"))(moduleName);
    if (mod && (mod.length || Object.keys(mod).length))
      return mod;
  } catch (t) {
  }
  return null;
}
var utf8$2 = {};
(function(t) {
  var e = t;
  e.length = function(s) {
    for (var o = 0, u = 0, n = 0; n < s.length; ++n)
      u = s.charCodeAt(n), u < 128 ? o += 1 : u < 2048 ? o += 2 : (u & 64512) === 55296 && (s.charCodeAt(n + 1) & 64512) === 56320 ? (++n, o += 4) : o += 3;
    return o;
  }, e.read = function(s, o, u) {
    var n = u - o;
    if (n < 1)
      return "";
    for (var a = null, f = [], i = 0, l; o < u; )
      l = s[o++], l < 128 ? f[i++] = l : l > 191 && l < 224 ? f[i++] = (l & 31) << 6 | s[o++] & 63 : l > 239 && l < 365 ? (l = ((l & 7) << 18 | (s[o++] & 63) << 12 | (s[o++] & 63) << 6 | s[o++] & 63) - 65536, f[i++] = 55296 + (l >> 10), f[i++] = 56320 + (l & 1023)) : f[i++] = (l & 15) << 12 | (s[o++] & 63) << 6 | s[o++] & 63, i > 8191 && ((a || (a = [])).push(String.fromCharCode.apply(String, f)), i = 0);
    return a ? (i && a.push(String.fromCharCode.apply(String, f.slice(0, i))), a.join("")) : String.fromCharCode.apply(String, f.slice(0, i));
  }, e.write = function(s, o, u) {
    for (var n = u, a, f, i = 0; i < s.length; ++i)
      a = s.charCodeAt(i), a < 128 ? o[u++] = a : a < 2048 ? (o[u++] = a >> 6 | 192, o[u++] = a & 63 | 128) : (a & 64512) === 55296 && ((f = s.charCodeAt(i + 1)) & 64512) === 56320 ? (a = 65536 + ((a & 1023) << 10) + (f & 1023), ++i, o[u++] = a >> 18 | 240, o[u++] = a >> 12 & 63 | 128, o[u++] = a >> 6 & 63 | 128, o[u++] = a & 63 | 128) : (o[u++] = a >> 12 | 224, o[u++] = a >> 6 & 63 | 128, o[u++] = a & 63 | 128);
    return u - n;
  };
})(utf8$2);
var pool_1 = pool;
function pool(t, e, r) {
  var s = r || 8192, o = s >>> 1, u = null, n = s;
  return function(f) {
    if (f < 1 || f > o)
      return t(f);
    n + f > s && (u = t(s), n = 0);
    var i = e.call(u, n, n += f);
    return n & 7 && (n = (n | 7) + 1), i;
  };
}
var longbits, hasRequiredLongbits;
function requireLongbits() {
  if (hasRequiredLongbits) return longbits;
  hasRequiredLongbits = 1, longbits = e;
  var t = requireMinimal();
  function e(u, n) {
    this.lo = u >>> 0, this.hi = n >>> 0;
  }
  var r = e.zero = new e(0, 0);
  r.toNumber = function() {
    return 0;
  }, r.zzEncode = r.zzDecode = function() {
    return this;
  }, r.length = function() {
    return 1;
  };
  var s = e.zeroHash = "\0\0\0\0\0\0\0\0";
  e.fromNumber = function(n) {
    if (n === 0)
      return r;
    var a = n < 0;
    a && (n = -n);
    var f = n >>> 0, i = (n - f) / 4294967296 >>> 0;
    return a && (i = ~i >>> 0, f = ~f >>> 0, ++f > 4294967295 && (f = 0, ++i > 4294967295 && (i = 0))), new e(f, i);
  }, e.from = function(n) {
    if (typeof n == "number")
      return e.fromNumber(n);
    if (t.isString(n))
      if (t.Long)
        n = t.Long.fromString(n);
      else
        return e.fromNumber(parseInt(n, 10));
    return n.low || n.high ? new e(n.low >>> 0, n.high >>> 0) : r;
  }, e.prototype.toNumber = function(n) {
    if (!n && this.hi >>> 31) {
      var a = ~this.lo + 1 >>> 0, f = ~this.hi >>> 0;
      return a || (f = f + 1 >>> 0), -(a + f * 4294967296);
    }
    return this.lo + this.hi * 4294967296;
  }, e.prototype.toLong = function(n) {
    return t.Long ? new t.Long(this.lo | 0, this.hi | 0, !!n) : { low: this.lo | 0, high: this.hi | 0, unsigned: !!n };
  };
  var o = String.prototype.charCodeAt;
  return e.fromHash = function(n) {
    return n === s ? r : new e(
      (o.call(n, 0) | o.call(n, 1) << 8 | o.call(n, 2) << 16 | o.call(n, 3) << 24) >>> 0,
      (o.call(n, 4) | o.call(n, 5) << 8 | o.call(n, 6) << 16 | o.call(n, 7) << 24) >>> 0
    );
  }, e.prototype.toHash = function() {
    return String.fromCharCode(
      this.lo & 255,
      this.lo >>> 8 & 255,
      this.lo >>> 16 & 255,
      this.lo >>> 24,
      this.hi & 255,
      this.hi >>> 8 & 255,
      this.hi >>> 16 & 255,
      this.hi >>> 24
    );
  }, e.prototype.zzEncode = function() {
    var n = this.hi >> 31;
    return this.hi = ((this.hi << 1 | this.lo >>> 31) ^ n) >>> 0, this.lo = (this.lo << 1 ^ n) >>> 0, this;
  }, e.prototype.zzDecode = function() {
    var n = -(this.lo & 1);
    return this.lo = ((this.lo >>> 1 | this.hi << 31) ^ n) >>> 0, this.hi = (this.hi >>> 1 ^ n) >>> 0, this;
  }, e.prototype.length = function() {
    var n = this.lo, a = (this.lo >>> 28 | this.hi << 4) >>> 0, f = this.hi >>> 24;
    return f === 0 ? a === 0 ? n < 16384 ? n < 128 ? 1 : 2 : n < 2097152 ? 3 : 4 : a < 16384 ? a < 128 ? 5 : 6 : a < 2097152 ? 7 : 8 : f < 128 ? 9 : 10;
  }, longbits;
}
var hasRequiredMinimal;
function requireMinimal() {
  return hasRequiredMinimal || (hasRequiredMinimal = 1, function(t) {
    var e = t;
    e.asPromise = aspromise, e.base64 = base64$1, e.EventEmitter = eventemitter, e.float = float, e.inquire = inquire_1, e.utf8 = utf8$2, e.pool = pool_1, e.LongBits = requireLongbits(), e.isNode = !!(typeof commonjsGlobal < "u" && commonjsGlobal && commonjsGlobal.process && commonjsGlobal.process.versions && commonjsGlobal.process.versions.node), e.global = e.isNode && commonjsGlobal || typeof window < "u" && window || typeof self < "u" && self || commonjsGlobal, e.emptyArray = Object.freeze ? Object.freeze([]) : (
      /* istanbul ignore next */
      []
    ), e.emptyObject = Object.freeze ? Object.freeze({}) : (
      /* istanbul ignore next */
      {}
    ), e.isInteger = Number.isInteger || /* istanbul ignore next */
    function(u) {
      return typeof u == "number" && isFinite(u) && Math.floor(u) === u;
    }, e.isString = function(u) {
      return typeof u == "string" || u instanceof String;
    }, e.isObject = function(u) {
      return u && typeof u == "object";
    }, e.isset = /**
     * Checks if a property on a message is considered to be present.
     * @param {Object} obj Plain object or message instance
     * @param {string} prop Property name
     * @returns {boolean} `true` if considered to be present, otherwise `false`
     */
    e.isSet = function(u, n) {
      var a = u[n];
      return a != null && u.hasOwnProperty(n) ? typeof a != "object" || (Array.isArray(a) ? a.length : Object.keys(a).length) > 0 : !1;
    }, e.Buffer = function() {
      try {
        var o = e.inquire("buffer").Buffer;
        return o.prototype.utf8Write ? o : (
          /* istanbul ignore next */
          null
        );
      } catch {
        return null;
      }
    }(), e._Buffer_from = null, e._Buffer_allocUnsafe = null, e.newBuffer = function(u) {
      return typeof u == "number" ? e.Buffer ? e._Buffer_allocUnsafe(u) : new e.Array(u) : e.Buffer ? e._Buffer_from(u) : typeof Uint8Array > "u" ? u : new Uint8Array(u);
    }, e.Array = typeof Uint8Array < "u" ? Uint8Array : Array, e.Long = /* istanbul ignore next */
    e.global.dcodeIO && /* istanbul ignore next */
    e.global.dcodeIO.Long || /* istanbul ignore next */
    e.global.Long || e.inquire("long"), e.key2Re = /^true|false|0|1$/, e.key32Re = /^-?(?:0|[1-9][0-9]*)$/, e.key64Re = /^(?:[\\x00-\\xff]{8}|-?(?:0|[1-9][0-9]*))$/, e.longToHash = function(u) {
      return u ? e.LongBits.from(u).toHash() : e.LongBits.zeroHash;
    }, e.longFromHash = function(u, n) {
      var a = e.LongBits.fromHash(u);
      return e.Long ? e.Long.fromBits(a.lo, a.hi, n) : a.toNumber(!!n);
    };
    function r(o, u, n) {
      for (var a = Object.keys(u), f = 0; f < a.length; ++f)
        (o[a[f]] === void 0 || !n) && (o[a[f]] = u[a[f]]);
      return o;
    }
    e.merge = r, e.lcFirst = function(u) {
      return u.charAt(0).toLowerCase() + u.substring(1);
    };
    function s(o) {
      function u(n, a) {
        if (!(this instanceof u))
          return new u(n, a);
        Object.defineProperty(this, "message", { get: function() {
          return n;
        } }), Error.captureStackTrace ? Error.captureStackTrace(this, u) : Object.defineProperty(this, "stack", { value: new Error().stack || "" }), a && r(this, a);
      }
      return u.prototype = Object.create(Error.prototype, {
        constructor: {
          value: u,
          writable: !0,
          enumerable: !1,
          configurable: !0
        },
        name: {
          get: function() {
            return o;
          },
          set: void 0,
          enumerable: !1,
          // configurable: false would accurately preserve the behavior of
          // the original, but I'm guessing that was not intentional.
          // For an actual error subclass, this property would
          // be configurable.
          configurable: !0
        },
        toString: {
          value: function() {
            return this.name + ": " + this.message;
          },
          writable: !0,
          enumerable: !1,
          configurable: !0
        }
      }), u;
    }
    e.newError = s, e.ProtocolError = s("ProtocolError"), e.oneOfGetter = function(u) {
      for (var n = {}, a = 0; a < u.length; ++a)
        n[u[a]] = 1;
      return function() {
        for (var f = Object.keys(this), i = f.length - 1; i > -1; --i)
          if (n[f[i]] === 1 && this[f[i]] !== void 0 && this[f[i]] !== null)
            return f[i];
      };
    }, e.oneOfSetter = function(u) {
      return function(n) {
        for (var a = 0; a < u.length; ++a)
          u[a] !== n && delete this[u[a]];
      };
    }, e.toJSONOptions = {
      longs: String,
      enums: String,
      bytes: String,
      json: !0
    }, e._configure = function() {
      var o = e.Buffer;
      if (!o) {
        e._Buffer_from = e._Buffer_allocUnsafe = null;
        return;
      }
      e._Buffer_from = o.from !== Uint8Array.from && o.from || /* istanbul ignore next */
      function(n, a) {
        return new o(n, a);
      }, e._Buffer_allocUnsafe = o.allocUnsafe || /* istanbul ignore next */
      function(n) {
        return new o(n);
      };
    };
  }(minimal)), minimal;
}
var writer = Writer$1, util$6 = requireMinimal(), BufferWriter$1, LongBits$1 = util$6.LongBits, base64 = util$6.base64, utf8$1 = util$6.utf8;
function Op(t, e, r) {
  this.fn = t, this.len = e, this.next = void 0, this.val = r;
}
function noop() {
}
function State(t) {
  this.head = t.head, this.tail = t.tail, this.len = t.len, this.next = t.states;
}
function Writer$1() {
  this.len = 0, this.head = new Op(noop, 0, 0), this.tail = this.head, this.states = null;
}
var create$1 = function t() {
  return util$6.Buffer ? function() {
    return (Writer$1.create = function() {
      return new BufferWriter$1();
    })();
  } : function() {
    return new Writer$1();
  };
};
Writer$1.create = create$1();
Writer$1.alloc = function t(e) {
  return new util$6.Array(e);
};
util$6.Array !== Array && (Writer$1.alloc = util$6.pool(Writer$1.alloc, util$6.Array.prototype.subarray));
Writer$1.prototype._push = function t(e, r, s) {
  return this.tail = this.tail.next = new Op(e, r, s), this.len += r, this;
};
function writeByte(t, e, r) {
  e[r] = t & 255;
}
function writeVarint32(t, e, r) {
  for (; t > 127; )
    e[r++] = t & 127 | 128, t >>>= 7;
  e[r] = t;
}
function VarintOp(t, e) {
  this.len = t, this.next = void 0, this.val = e;
}
VarintOp.prototype = Object.create(Op.prototype);
VarintOp.prototype.fn = writeVarint32;
Writer$1.prototype.uint32 = function t(e) {
  return this.len += (this.tail = this.tail.next = new VarintOp(
    (e = e >>> 0) < 128 ? 1 : e < 16384 ? 2 : e < 2097152 ? 3 : e < 268435456 ? 4 : 5,
    e
  )).len, this;
};
Writer$1.prototype.int32 = function t(e) {
  return e < 0 ? this._push(writeVarint64, 10, LongBits$1.fromNumber(e)) : this.uint32(e);
};
Writer$1.prototype.sint32 = function t(e) {
  return this.uint32((e << 1 ^ e >> 31) >>> 0);
};
function writeVarint64(t, e, r) {
  for (; t.hi; )
    e[r++] = t.lo & 127 | 128, t.lo = (t.lo >>> 7 | t.hi << 25) >>> 0, t.hi >>>= 7;
  for (; t.lo > 127; )
    e[r++] = t.lo & 127 | 128, t.lo = t.lo >>> 7;
  e[r++] = t.lo;
}
Writer$1.prototype.uint64 = function t(e) {
  var r = LongBits$1.from(e);
  return this._push(writeVarint64, r.length(), r);
};
Writer$1.prototype.int64 = Writer$1.prototype.uint64;
Writer$1.prototype.sint64 = function t(e) {
  var r = LongBits$1.from(e).zzEncode();
  return this._push(writeVarint64, r.length(), r);
};
Writer$1.prototype.bool = function t(e) {
  return this._push(writeByte, 1, e ? 1 : 0);
};
function writeFixed32(t, e, r) {
  e[r] = t & 255, e[r + 1] = t >>> 8 & 255, e[r + 2] = t >>> 16 & 255, e[r + 3] = t >>> 24;
}
Writer$1.prototype.fixed32 = function t(e) {
  return this._push(writeFixed32, 4, e >>> 0);
};
Writer$1.prototype.sfixed32 = Writer$1.prototype.fixed32;
Writer$1.prototype.fixed64 = function t(e) {
  var r = LongBits$1.from(e);
  return this._push(writeFixed32, 4, r.lo)._push(writeFixed32, 4, r.hi);
};
Writer$1.prototype.sfixed64 = Writer$1.prototype.fixed64;
Writer$1.prototype.float = function t(e) {
  return this._push(util$6.float.writeFloatLE, 4, e);
};
Writer$1.prototype.double = function t(e) {
  return this._push(util$6.float.writeDoubleLE, 8, e);
};
var writeBytes = util$6.Array.prototype.set ? function t(e, r, s) {
  r.set(e, s);
} : function t(e, r, s) {
  for (var o = 0; o < e.length; ++o)
    r[s + o] = e[o];
};
Writer$1.prototype.bytes = function t(e) {
  var r = e.length >>> 0;
  if (!r)
    return this._push(writeByte, 1, 0);
  if (util$6.isString(e)) {
    var s = Writer$1.alloc(r = base64.length(e));
    base64.decode(e, s, 0), e = s;
  }
  return this.uint32(r)._push(writeBytes, r, e);
};
Writer$1.prototype.string = function t(e) {
  var r = utf8$1.length(e);
  return r ? this.uint32(r)._push(utf8$1.write, r, e) : this._push(writeByte, 1, 0);
};
Writer$1.prototype.fork = function t() {
  return this.states = new State(this), this.head = this.tail = new Op(noop, 0, 0), this.len = 0, this;
};
Writer$1.prototype.reset = function t() {
  return this.states ? (this.head = this.states.head, this.tail = this.states.tail, this.len = this.states.len, this.states = this.states.next) : (this.head = this.tail = new Op(noop, 0, 0), this.len = 0), this;
};
Writer$1.prototype.ldelim = function t() {
  var e = this.head, r = this.tail, s = this.len;
  return this.reset().uint32(s), s && (this.tail.next = e.next, this.tail = r, this.len += s), this;
};
Writer$1.prototype.finish = function t() {
  for (var e = this.head.next, r = this.constructor.alloc(this.len), s = 0; e; )
    e.fn(e.val, r, s), s += e.len, e = e.next;
  return r;
};
Writer$1._configure = function(t) {
  BufferWriter$1 = t, Writer$1.create = create$1(), BufferWriter$1._configure();
};
var writer_buffer = BufferWriter, Writer = writer;
(BufferWriter.prototype = Object.create(Writer.prototype)).constructor = BufferWriter;
var util$5 = requireMinimal();
function BufferWriter() {
  Writer.call(this);
}
BufferWriter._configure = function() {
  BufferWriter.alloc = util$5._Buffer_allocUnsafe, BufferWriter.writeBytesBuffer = util$5.Buffer && util$5.Buffer.prototype instanceof Uint8Array && util$5.Buffer.prototype.set.name === "set" ? function(e, r, s) {
    r.set(e, s);
  } : function(e, r, s) {
    if (e.copy)
      e.copy(r, s, 0, e.length);
    else for (var o = 0; o < e.length; )
      r[s++] = e[o++];
  };
};
BufferWriter.prototype.bytes = function t(e) {
  util$5.isString(e) && (e = util$5._Buffer_from(e, "base64"));
  var r = e.length >>> 0;
  return this.uint32(r), r && this._push(BufferWriter.writeBytesBuffer, r, e), this;
};
function writeStringBuffer(t, e, r) {
  t.length < 40 ? util$5.utf8.write(t, e, r) : e.utf8Write ? e.utf8Write(t, r) : e.write(t, r);
}
BufferWriter.prototype.string = function t(e) {
  var r = util$5.Buffer.byteLength(e);
  return this.uint32(r), r && this._push(writeStringBuffer, r, e), this;
};
BufferWriter._configure();
var reader = Reader$1, util$4 = requireMinimal(), BufferReader$1, LongBits = util$4.LongBits, utf8 = util$4.utf8;
function indexOutOfRange(t, e) {
  return RangeError("index out of range: " + t.pos + " + " + (e || 1) + " > " + t.len);
}
function Reader$1(t) {
  this.buf = t, this.pos = 0, this.len = t.length;
}
var create_array = typeof Uint8Array < "u" ? function t(e) {
  if (e instanceof Uint8Array || Array.isArray(e))
    return new Reader$1(e);
  throw Error("illegal buffer");
} : function t(e) {
  if (Array.isArray(e))
    return new Reader$1(e);
  throw Error("illegal buffer");
}, create = function t() {
  return util$4.Buffer ? function(r) {
    return (Reader$1.create = function(o) {
      return util$4.Buffer.isBuffer(o) ? new BufferReader$1(o) : create_array(o);
    })(r);
  } : create_array;
};
Reader$1.create = create();
Reader$1.prototype._slice = util$4.Array.prototype.subarray || /* istanbul ignore next */
util$4.Array.prototype.slice;
Reader$1.prototype.uint32 = /* @__PURE__ */ function t() {
  var e = 4294967295;
  return function() {
    if (e = (this.buf[this.pos] & 127) >>> 0, this.buf[this.pos++] < 128 || (e = (e | (this.buf[this.pos] & 127) << 7) >>> 0, this.buf[this.pos++] < 128) || (e = (e | (this.buf[this.pos] & 127) << 14) >>> 0, this.buf[this.pos++] < 128) || (e = (e | (this.buf[this.pos] & 127) << 21) >>> 0, this.buf[this.pos++] < 128) || (e = (e | (this.buf[this.pos] & 15) << 28) >>> 0, this.buf[this.pos++] < 128)) return e;
    if ((this.pos += 5) > this.len)
      throw this.pos = this.len, indexOutOfRange(this, 10);
    return e;
  };
}();
Reader$1.prototype.int32 = function t() {
  return this.uint32() | 0;
};
Reader$1.prototype.sint32 = function t() {
  var e = this.uint32();
  return e >>> 1 ^ -(e & 1) | 0;
};
function readLongVarint() {
  var t = new LongBits(0, 0), e = 0;
  if (this.len - this.pos > 4) {
    for (; e < 4; ++e)
      if (t.lo = (t.lo | (this.buf[this.pos] & 127) << e * 7) >>> 0, this.buf[this.pos++] < 128)
        return t;
    if (t.lo = (t.lo | (this.buf[this.pos] & 127) << 28) >>> 0, t.hi = (t.hi | (this.buf[this.pos] & 127) >> 4) >>> 0, this.buf[this.pos++] < 128)
      return t;
    e = 0;
  } else {
    for (; e < 3; ++e) {
      if (this.pos >= this.len)
        throw indexOutOfRange(this);
      if (t.lo = (t.lo | (this.buf[this.pos] & 127) << e * 7) >>> 0, this.buf[this.pos++] < 128)
        return t;
    }
    return t.lo = (t.lo | (this.buf[this.pos++] & 127) << e * 7) >>> 0, t;
  }
  if (this.len - this.pos > 4) {
    for (; e < 5; ++e)
      if (t.hi = (t.hi | (this.buf[this.pos] & 127) << e * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
        return t;
  } else
    for (; e < 5; ++e) {
      if (this.pos >= this.len)
        throw indexOutOfRange(this);
      if (t.hi = (t.hi | (this.buf[this.pos] & 127) << e * 7 + 3) >>> 0, this.buf[this.pos++] < 128)
        return t;
    }
  throw Error("invalid varint encoding");
}
Reader$1.prototype.bool = function t() {
  return this.uint32() !== 0;
};
function readFixed32_end(t, e) {
  return (t[e - 4] | t[e - 3] << 8 | t[e - 2] << 16 | t[e - 1] << 24) >>> 0;
}
Reader$1.prototype.fixed32 = function t() {
  if (this.pos + 4 > this.len)
    throw indexOutOfRange(this, 4);
  return readFixed32_end(this.buf, this.pos += 4);
};
Reader$1.prototype.sfixed32 = function t() {
  if (this.pos + 4 > this.len)
    throw indexOutOfRange(this, 4);
  return readFixed32_end(this.buf, this.pos += 4) | 0;
};
function readFixed64() {
  if (this.pos + 8 > this.len)
    throw indexOutOfRange(this, 8);
  return new LongBits(readFixed32_end(this.buf, this.pos += 4), readFixed32_end(this.buf, this.pos += 4));
}
Reader$1.prototype.float = function t() {
  if (this.pos + 4 > this.len)
    throw indexOutOfRange(this, 4);
  var e = util$4.float.readFloatLE(this.buf, this.pos);
  return this.pos += 4, e;
};
Reader$1.prototype.double = function t() {
  if (this.pos + 8 > this.len)
    throw indexOutOfRange(this, 4);
  var e = util$4.float.readDoubleLE(this.buf, this.pos);
  return this.pos += 8, e;
};
Reader$1.prototype.bytes = function t() {
  var e = this.uint32(), r = this.pos, s = this.pos + e;
  if (s > this.len)
    throw indexOutOfRange(this, e);
  if (this.pos += e, Array.isArray(this.buf))
    return this.buf.slice(r, s);
  if (r === s) {
    var o = util$4.Buffer;
    return o ? o.alloc(0) : new this.buf.constructor(0);
  }
  return this._slice.call(this.buf, r, s);
};
Reader$1.prototype.string = function t() {
  var e = this.bytes();
  return utf8.read(e, 0, e.length);
};
Reader$1.prototype.skip = function t(e) {
  if (typeof e == "number") {
    if (this.pos + e > this.len)
      throw indexOutOfRange(this, e);
    this.pos += e;
  } else
    do
      if (this.pos >= this.len)
        throw indexOutOfRange(this);
    while (this.buf[this.pos++] & 128);
  return this;
};
Reader$1.prototype.skipType = function(t) {
  switch (t) {
    case 0:
      this.skip();
      break;
    case 1:
      this.skip(8);
      break;
    case 2:
      this.skip(this.uint32());
      break;
    case 3:
      for (; (t = this.uint32() & 7) !== 4; )
        this.skipType(t);
      break;
    case 5:
      this.skip(4);
      break;
    default:
      throw Error("invalid wire type " + t + " at offset " + this.pos);
  }
  return this;
};
Reader$1._configure = function(t) {
  BufferReader$1 = t, Reader$1.create = create(), BufferReader$1._configure();
  var e = util$4.Long ? "toLong" : (
    /* istanbul ignore next */
    "toNumber"
  );
  util$4.merge(Reader$1.prototype, {
    int64: function() {
      return readLongVarint.call(this)[e](!1);
    },
    uint64: function() {
      return readLongVarint.call(this)[e](!0);
    },
    sint64: function() {
      return readLongVarint.call(this).zzDecode()[e](!1);
    },
    fixed64: function() {
      return readFixed64.call(this)[e](!0);
    },
    sfixed64: function() {
      return readFixed64.call(this)[e](!1);
    }
  });
};
var reader_buffer = BufferReader, Reader = reader;
(BufferReader.prototype = Object.create(Reader.prototype)).constructor = BufferReader;
var util$3 = requireMinimal();
function BufferReader(t) {
  Reader.call(this, t);
}
BufferReader._configure = function() {
  util$3.Buffer && (BufferReader.prototype._slice = util$3.Buffer.prototype.slice);
};
BufferReader.prototype.string = function t() {
  var e = this.uint32();
  return this.buf.utf8Slice ? this.buf.utf8Slice(this.pos, this.pos = Math.min(this.pos + e, this.len)) : this.buf.toString("utf-8", this.pos, this.pos = Math.min(this.pos + e, this.len));
};
BufferReader._configure();
var rpc = {}, service$1 = Service, util$2 = requireMinimal();
(Service.prototype = Object.create(util$2.EventEmitter.prototype)).constructor = Service;
function Service(t, e, r) {
  if (typeof t != "function")
    throw TypeError("rpcImpl must be a function");
  util$2.EventEmitter.call(this), this.rpcImpl = t, this.requestDelimited = !!e, this.responseDelimited = !!r;
}
Service.prototype.rpcCall = function t(e, r, s, o, u) {
  if (!o)
    throw TypeError("request must be specified");
  var n = this;
  if (!u)
    return util$2.asPromise(t, n, e, r, s, o);
  if (!n.rpcImpl) {
    setTimeout(function() {
      u(Error("already ended"));
    }, 0);
    return;
  }
  try {
    return n.rpcImpl(
      e,
      r[n.requestDelimited ? "encodeDelimited" : "encode"](o).finish(),
      function(f, i) {
        if (f)
          return n.emit("error", f, e), u(f);
        if (i === null) {
          n.end(
            /* endedByRPC */
            !0
          );
          return;
        }
        if (!(i instanceof s))
          try {
            i = s[n.responseDelimited ? "decodeDelimited" : "decode"](i);
          } catch (l) {
            return n.emit("error", l, e), u(l);
          }
        return n.emit("data", i, e), u(null, i);
      }
    );
  } catch (a) {
    n.emit("error", a, e), setTimeout(function() {
      u(a);
    }, 0);
    return;
  }
};
Service.prototype.end = function t(e) {
  return this.rpcImpl && (e || this.rpcImpl(null, null, null), this.rpcImpl = null, this.emit("end").off()), this;
};
(function(t) {
  var e = t;
  e.Service = service$1;
})(rpc);
var roots = {};
(function(t) {
  var e = t;
  e.build = "minimal", e.Writer = writer, e.BufferWriter = writer_buffer, e.Reader = reader, e.BufferReader = reader_buffer, e.util = requireMinimal(), e.rpc = rpc, e.roots = roots, e.configure = r;
  function r() {
    e.util._configure(), e.Writer._configure(e.BufferWriter), e.Reader._configure(e.BufferReader);
  }
  r();
})(indexMinimal);
var util$1 = { exports: {} }, codegen_1 = codegen;
function codegen(t, e) {
  typeof t == "string" && (e = t, t = void 0);
  var r = [];
  function s(u) {
    if (typeof u != "string") {
      var n = o();
      if (codegen.verbose && console.log("codegen: " + n), n = "return " + n, u) {
        for (var a = Object.keys(u), f = new Array(a.length + 1), i = new Array(a.length), l = 0; l < a.length; )
          f[l] = a[l], i[l] = u[a[l++]];
        return f[l] = n, Function.apply(null, f).apply(null, i);
      }
      return Function(n)();
    }
    for (var c = new Array(arguments.length - 1), h = 0; h < c.length; )
      c[h] = arguments[++h];
    if (h = 0, u = u.replace(/%([%dfijs])/g, function(p, v) {
      var g = c[h++];
      switch (v) {
        case "d":
        case "f":
          return String(Number(g));
        case "i":
          return String(Math.floor(g));
        case "j":
          return JSON.stringify(g);
        case "s":
          return String(g);
      }
      return "%";
    }), h !== c.length)
      throw Error("parameter count mismatch");
    return r.push(u), s;
  }
  function o(u) {
    return "function " + (u || e || "") + "(" + (t && t.join(",") || "") + `){
  ` + r.join(`
  `) + `
}`;
  }
  return s.toString = o, s;
}
codegen.verbose = !1;
var fetch_1 = fetch, asPromise = aspromise, inquire = inquire_1, fs = inquire("fs");
function fetch(t, e, r) {
  return typeof e == "function" ? (r = e, e = {}) : e || (e = {}), r ? !e.xhr && fs && fs.readFile ? fs.readFile(t, function(o, u) {
    return o && typeof XMLHttpRequest < "u" ? fetch.xhr(t, e, r) : o ? r(o) : r(null, e.binary ? u : u.toString("utf8"));
  }) : fetch.xhr(t, e, r) : asPromise(fetch, this, t, e);
}
fetch.xhr = function t(e, r, s) {
  var o = new XMLHttpRequest();
  o.onreadystatechange = function() {
    if (o.readyState === 4) {
      if (o.status !== 0 && o.status !== 200)
        return s(Error("status " + o.status));
      if (r.binary) {
        var n = o.response;
        if (!n) {
          n = [];
          for (var a = 0; a < o.responseText.length; ++a)
            n.push(o.responseText.charCodeAt(a) & 255);
        }
        return s(null, typeof Uint8Array < "u" ? new Uint8Array(n) : n);
      }
      return s(null, o.responseText);
    }
  }, r.binary && ("overrideMimeType" in o && o.overrideMimeType("text/plain; charset=x-user-defined"), o.responseType = "arraybuffer"), o.open("GET", e), o.send();
};
var path = {};
(function(t) {
  var e = t, r = (
    /**
     * Tests if the specified path is absolute.
     * @param {string} path Path to test
     * @returns {boolean} `true` if path is absolute
     */
    e.isAbsolute = function(u) {
      return /^(?:\/|\w+:)/.test(u);
    }
  ), s = (
    /**
     * Normalizes the specified path.
     * @param {string} path Path to normalize
     * @returns {string} Normalized path
     */
    e.normalize = function(u) {
      u = u.replace(/\\/g, "/").replace(/\/{2,}/g, "/");
      var n = u.split("/"), a = r(u), f = "";
      a && (f = n.shift() + "/");
      for (var i = 0; i < n.length; )
        n[i] === ".." ? i > 0 && n[i - 1] !== ".." ? n.splice(--i, 2) : a ? n.splice(i, 1) : ++i : n[i] === "." ? n.splice(i, 1) : ++i;
      return f + n.join("/");
    }
  );
  e.resolve = function(u, n, a) {
    return a || (n = s(n)), r(n) ? n : (a || (u = s(u)), (u = u.replace(/(?:\/|^)[^/]+$/, "")).length ? s(u + "/" + n) : n);
  };
})(path);
var types = {}, hasRequiredTypes;
function requireTypes() {
  return hasRequiredTypes || (hasRequiredTypes = 1, function(t) {
    var e = t, r = requireUtil(), s = [
      "double",
      // 0
      "float",
      // 1
      "int32",
      // 2
      "uint32",
      // 3
      "sint32",
      // 4
      "fixed32",
      // 5
      "sfixed32",
      // 6
      "int64",
      // 7
      "uint64",
      // 8
      "sint64",
      // 9
      "fixed64",
      // 10
      "sfixed64",
      // 11
      "bool",
      // 12
      "string",
      // 13
      "bytes"
      // 14
    ];
    function o(u, n) {
      var a = 0, f = {};
      for (n |= 0; a < u.length; ) f[s[a + n]] = u[a++];
      return f;
    }
    e.basic = o([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2,
      /* bytes    */
      2
    ]), e.defaults = o([
      /* double   */
      0,
      /* float    */
      0,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      0,
      /* sfixed32 */
      0,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      0,
      /* sfixed64 */
      0,
      /* bool     */
      !1,
      /* string   */
      "",
      /* bytes    */
      r.emptyArray,
      /* message  */
      null
    ]), e.long = o([
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1
    ], 7), e.mapKey = o([
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0,
      /* string   */
      2
    ], 2), e.packed = o([
      /* double   */
      1,
      /* float    */
      5,
      /* int32    */
      0,
      /* uint32   */
      0,
      /* sint32   */
      0,
      /* fixed32  */
      5,
      /* sfixed32 */
      5,
      /* int64    */
      0,
      /* uint64   */
      0,
      /* sint64   */
      0,
      /* fixed64  */
      1,
      /* sfixed64 */
      1,
      /* bool     */
      0
    ]);
  }(types)), types;
}
var field, hasRequiredField;
function requireField() {
  if (hasRequiredField) return field;
  hasRequiredField = 1, field = n;
  var t = requireObject();
  ((n.prototype = Object.create(t.prototype)).constructor = n).className = "Field";
  var e = require_enum(), r = requireTypes(), s = requireUtil(), o, u = /^required|optional|repeated$/;
  n.fromJSON = function(f, i) {
    return new n(f, i.id, i.type, i.rule, i.extend, i.options, i.comment);
  };
  function n(a, f, i, l, c, h, d) {
    if (s.isObject(l) ? (d = c, h = l, l = c = void 0) : s.isObject(c) && (d = h, h = c, c = void 0), t.call(this, a, h), !s.isInteger(f) || f < 0)
      throw TypeError("id must be a non-negative integer");
    if (!s.isString(i))
      throw TypeError("type must be a string");
    if (l !== void 0 && !u.test(l = l.toString().toLowerCase()))
      throw TypeError("rule must be a string rule");
    if (c !== void 0 && !s.isString(c))
      throw TypeError("extend must be a string");
    l === "proto3_optional" && (l = "optional"), this.rule = l && l !== "optional" ? l : void 0, this.type = i, this.id = f, this.extend = c || void 0, this.required = l === "required", this.optional = !this.required, this.repeated = l === "repeated", this.map = !1, this.message = null, this.partOf = null, this.typeDefault = null, this.defaultValue = null, this.long = s.Long ? r.long[i] !== void 0 : (
      /* istanbul ignore next */
      !1
    ), this.bytes = i === "bytes", this.resolvedType = null, this.extensionField = null, this.declaringField = null, this._packed = null, this.comment = d;
  }
  return Object.defineProperty(n.prototype, "packed", {
    get: function() {
      return this._packed === null && (this._packed = this.getOption("packed") !== !1), this._packed;
    }
  }), n.prototype.setOption = function(f, i, l) {
    return f === "packed" && (this._packed = null), t.prototype.setOption.call(this, f, i, l);
  }, n.prototype.toJSON = function(f) {
    var i = f ? !!f.keepComments : !1;
    return s.toObject([
      "rule",
      this.rule !== "optional" && this.rule || void 0,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      i ? this.comment : void 0
    ]);
  }, n.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if ((this.typeDefault = r.defaults[this.type]) === void 0 ? (this.resolvedType = (this.declaringField ? this.declaringField.parent : this.parent).lookupTypeOrEnum(this.type), this.resolvedType instanceof o ? this.typeDefault = null : this.typeDefault = this.resolvedType.values[Object.keys(this.resolvedType.values)[0]]) : this.options && this.options.proto3_optional && (this.typeDefault = null), this.options && this.options.default != null && (this.typeDefault = this.options.default, this.resolvedType instanceof e && typeof this.typeDefault == "string" && (this.typeDefault = this.resolvedType.values[this.typeDefault])), this.options && ((this.options.packed === !0 || this.options.packed !== void 0 && this.resolvedType && !(this.resolvedType instanceof e)) && delete this.options.packed, Object.keys(this.options).length || (this.options = void 0)), this.long)
      this.typeDefault = s.Long.fromNumber(this.typeDefault, this.type.charAt(0) === "u"), Object.freeze && Object.freeze(this.typeDefault);
    else if (this.bytes && typeof this.typeDefault == "string") {
      var f;
      s.base64.test(this.typeDefault) ? s.base64.decode(this.typeDefault, f = s.newBuffer(s.base64.length(this.typeDefault)), 0) : s.utf8.write(this.typeDefault, f = s.newBuffer(s.utf8.length(this.typeDefault)), 0), this.typeDefault = f;
    }
    return this.map ? this.defaultValue = s.emptyObject : this.repeated ? this.defaultValue = s.emptyArray : this.defaultValue = this.typeDefault, this.parent instanceof o && (this.parent.ctor.prototype[this.name] = this.defaultValue), t.prototype.resolve.call(this);
  }, n.d = function(f, i, l, c) {
    return typeof i == "function" ? i = s.decorateType(i).name : i && typeof i == "object" && (i = s.decorateEnum(i).name), function(d, p) {
      s.decorateType(d.constructor).add(new n(p, f, i, l, { default: c }));
    };
  }, n._configure = function(f) {
    o = f;
  }, field;
}
var oneof, hasRequiredOneof;
function requireOneof() {
  if (hasRequiredOneof) return oneof;
  hasRequiredOneof = 1, oneof = s;
  var t = requireObject();
  ((s.prototype = Object.create(t.prototype)).constructor = s).className = "OneOf";
  var e = requireField(), r = requireUtil();
  function s(u, n, a, f) {
    if (Array.isArray(n) || (a = n, n = void 0), t.call(this, u, a), !(n === void 0 || Array.isArray(n)))
      throw TypeError("fieldNames must be an Array");
    this.oneof = n || [], this.fieldsArray = [], this.comment = f;
  }
  s.fromJSON = function(n, a) {
    return new s(n, a.oneof, a.options, a.comment);
  }, s.prototype.toJSON = function(n) {
    var a = n ? !!n.keepComments : !1;
    return r.toObject([
      "options",
      this.options,
      "oneof",
      this.oneof,
      "comment",
      a ? this.comment : void 0
    ]);
  };
  function o(u) {
    if (u.parent)
      for (var n = 0; n < u.fieldsArray.length; ++n)
        u.fieldsArray[n].parent || u.parent.add(u.fieldsArray[n]);
  }
  return s.prototype.add = function(n) {
    if (!(n instanceof e))
      throw TypeError("field must be a Field");
    return n.parent && n.parent !== this.parent && n.parent.remove(n), this.oneof.push(n.name), this.fieldsArray.push(n), n.partOf = this, o(this), this;
  }, s.prototype.remove = function(n) {
    if (!(n instanceof e))
      throw TypeError("field must be a Field");
    var a = this.fieldsArray.indexOf(n);
    if (a < 0)
      throw Error(n + " is not a member of " + this);
    return this.fieldsArray.splice(a, 1), a = this.oneof.indexOf(n.name), a > -1 && this.oneof.splice(a, 1), n.partOf = null, this;
  }, s.prototype.onAdd = function(n) {
    t.prototype.onAdd.call(this, n);
    for (var a = this, f = 0; f < this.oneof.length; ++f) {
      var i = n.get(this.oneof[f]);
      i && !i.partOf && (i.partOf = a, a.fieldsArray.push(i));
    }
    o(this);
  }, s.prototype.onRemove = function(n) {
    for (var a = 0, f; a < this.fieldsArray.length; ++a)
      (f = this.fieldsArray[a]).parent && f.parent.remove(f);
    t.prototype.onRemove.call(this, n);
  }, s.d = function() {
    for (var n = new Array(arguments.length), a = 0; a < arguments.length; )
      n[a] = arguments[a++];
    return function(i, l) {
      r.decorateType(i.constructor).add(new s(l, n)), Object.defineProperty(i, l, {
        get: r.oneOfGetter(n),
        set: r.oneOfSetter(n)
      });
    };
  }, oneof;
}
var namespace, hasRequiredNamespace;
function requireNamespace() {
  if (hasRequiredNamespace) return namespace;
  hasRequiredNamespace = 1, namespace = f;
  var t = requireObject();
  ((f.prototype = Object.create(t.prototype)).constructor = f).className = "Namespace";
  var e = requireField(), r = requireUtil(), s = requireOneof(), o, u, n;
  f.fromJSON = function(c, h) {
    return new f(c, h.options).addJSON(h.nested);
  };
  function a(l, c) {
    if (l && l.length) {
      for (var h = {}, d = 0; d < l.length; ++d)
        h[l[d].name] = l[d].toJSON(c);
      return h;
    }
  }
  f.arrayToJSON = a, f.isReservedId = function(c, h) {
    if (c) {
      for (var d = 0; d < c.length; ++d)
        if (typeof c[d] != "string" && c[d][0] <= h && c[d][1] > h)
          return !0;
    }
    return !1;
  }, f.isReservedName = function(c, h) {
    if (c) {
      for (var d = 0; d < c.length; ++d)
        if (c[d] === h)
          return !0;
    }
    return !1;
  };
  function f(l, c) {
    t.call(this, l, c), this.nested = void 0, this._nestedArray = null;
  }
  function i(l) {
    return l._nestedArray = null, l;
  }
  return Object.defineProperty(f.prototype, "nestedArray", {
    get: function() {
      return this._nestedArray || (this._nestedArray = r.toArray(this.nested));
    }
  }), f.prototype.toJSON = function(c) {
    return r.toObject([
      "options",
      this.options,
      "nested",
      a(this.nestedArray, c)
    ]);
  }, f.prototype.addJSON = function(c) {
    var h = this;
    if (c)
      for (var d = Object.keys(c), p = 0, v; p < d.length; ++p)
        v = c[d[p]], h.add(
          // most to least likely
          (v.fields !== void 0 ? o.fromJSON : v.values !== void 0 ? n.fromJSON : v.methods !== void 0 ? u.fromJSON : v.id !== void 0 ? e.fromJSON : f.fromJSON)(d[p], v)
        );
    return this;
  }, f.prototype.get = function(c) {
    return this.nested && this.nested[c] || null;
  }, f.prototype.getEnum = function(c) {
    if (this.nested && this.nested[c] instanceof n)
      return this.nested[c].values;
    throw Error("no such enum: " + c);
  }, f.prototype.add = function(c) {
    if (!(c instanceof e && c.extend !== void 0 || c instanceof o || c instanceof s || c instanceof n || c instanceof u || c instanceof f))
      throw TypeError("object must be a valid nested object");
    if (!this.nested)
      this.nested = {};
    else {
      var h = this.get(c.name);
      if (h)
        if (h instanceof f && c instanceof f && !(h instanceof o || h instanceof u)) {
          for (var d = h.nestedArray, p = 0; p < d.length; ++p)
            c.add(d[p]);
          this.remove(h), this.nested || (this.nested = {}), c.setOptions(h.options, !0);
        } else
          throw Error("duplicate name '" + c.name + "' in " + this);
    }
    return this.nested[c.name] = c, c.onAdd(this), i(this);
  }, f.prototype.remove = function(c) {
    if (!(c instanceof t))
      throw TypeError("object must be a ReflectionObject");
    if (c.parent !== this)
      throw Error(c + " is not a member of " + this);
    return delete this.nested[c.name], Object.keys(this.nested).length || (this.nested = void 0), c.onRemove(this), i(this);
  }, f.prototype.define = function(c, h) {
    if (r.isString(c))
      c = c.split(".");
    else if (!Array.isArray(c))
      throw TypeError("illegal path");
    if (c && c.length && c[0] === "")
      throw Error("path must be relative");
    for (var d = this; c.length > 0; ) {
      var p = c.shift();
      if (d.nested && d.nested[p]) {
        if (d = d.nested[p], !(d instanceof f))
          throw Error("path conflicts with non-namespace objects");
      } else
        d.add(d = new f(p));
    }
    return h && d.addJSON(h), d;
  }, f.prototype.resolveAll = function() {
    for (var c = this.nestedArray, h = 0; h < c.length; )
      c[h] instanceof f ? c[h++].resolveAll() : c[h++].resolve();
    return this.resolve();
  }, f.prototype.lookup = function(c, h, d) {
    if (typeof h == "boolean" ? (d = h, h = void 0) : h && !Array.isArray(h) && (h = [h]), r.isString(c) && c.length) {
      if (c === ".")
        return this.root;
      c = c.split(".");
    } else if (!c.length)
      return this;
    if (c[0] === "")
      return this.root.lookup(c.slice(1), h);
    var p = this.get(c[0]);
    if (p) {
      if (c.length === 1) {
        if (!h || h.indexOf(p.constructor) > -1)
          return p;
      } else if (p instanceof f && (p = p.lookup(c.slice(1), h, !0)))
        return p;
    } else
      for (var v = 0; v < this.nestedArray.length; ++v)
        if (this._nestedArray[v] instanceof f && (p = this._nestedArray[v].lookup(c, h, !0)))
          return p;
    return this.parent === null || d ? null : this.parent.lookup(c, h);
  }, f.prototype.lookupType = function(c) {
    var h = this.lookup(c, [o]);
    if (!h)
      throw Error("no such type: " + c);
    return h;
  }, f.prototype.lookupEnum = function(c) {
    var h = this.lookup(c, [n]);
    if (!h)
      throw Error("no such Enum '" + c + "' in " + this);
    return h;
  }, f.prototype.lookupTypeOrEnum = function(c) {
    var h = this.lookup(c, [o, n]);
    if (!h)
      throw Error("no such Type or Enum '" + c + "' in " + this);
    return h;
  }, f.prototype.lookupService = function(c) {
    var h = this.lookup(c, [u]);
    if (!h)
      throw Error("no such Service '" + c + "' in " + this);
    return h;
  }, f._configure = function(l, c, h) {
    o = l, u = c, n = h;
  }, namespace;
}
var mapfield, hasRequiredMapfield;
function requireMapfield() {
  if (hasRequiredMapfield) return mapfield;
  hasRequiredMapfield = 1, mapfield = s;
  var t = requireField();
  ((s.prototype = Object.create(t.prototype)).constructor = s).className = "MapField";
  var e = requireTypes(), r = requireUtil();
  function s(o, u, n, a, f, i) {
    if (t.call(this, o, u, a, void 0, void 0, f, i), !r.isString(n))
      throw TypeError("keyType must be a string");
    this.keyType = n, this.resolvedKeyType = null, this.map = !0;
  }
  return s.fromJSON = function(u, n) {
    return new s(u, n.id, n.keyType, n.type, n.options, n.comment);
  }, s.prototype.toJSON = function(u) {
    var n = u ? !!u.keepComments : !1;
    return r.toObject([
      "keyType",
      this.keyType,
      "type",
      this.type,
      "id",
      this.id,
      "extend",
      this.extend,
      "options",
      this.options,
      "comment",
      n ? this.comment : void 0
    ]);
  }, s.prototype.resolve = function() {
    if (this.resolved)
      return this;
    if (e.mapKey[this.keyType] === void 0)
      throw Error("invalid key type: " + this.keyType);
    return t.prototype.resolve.call(this);
  }, s.d = function(u, n, a) {
    return typeof a == "function" ? a = r.decorateType(a).name : a && typeof a == "object" && (a = r.decorateEnum(a).name), function(i, l) {
      r.decorateType(i.constructor).add(new s(l, u, n, a));
    };
  }, mapfield;
}
var method, hasRequiredMethod;
function requireMethod() {
  if (hasRequiredMethod) return method;
  hasRequiredMethod = 1, method = r;
  var t = requireObject();
  ((r.prototype = Object.create(t.prototype)).constructor = r).className = "Method";
  var e = requireUtil();
  function r(s, o, u, n, a, f, i, l, c) {
    if (e.isObject(a) ? (i = a, a = f = void 0) : e.isObject(f) && (i = f, f = void 0), !(o === void 0 || e.isString(o)))
      throw TypeError("type must be a string");
    if (!e.isString(u))
      throw TypeError("requestType must be a string");
    if (!e.isString(n))
      throw TypeError("responseType must be a string");
    t.call(this, s, i), this.type = o || "rpc", this.requestType = u, this.requestStream = a ? !0 : void 0, this.responseType = n, this.responseStream = f ? !0 : void 0, this.resolvedRequestType = null, this.resolvedResponseType = null, this.comment = l, this.parsedOptions = c;
  }
  return r.fromJSON = function(o, u) {
    return new r(o, u.type, u.requestType, u.responseType, u.requestStream, u.responseStream, u.options, u.comment, u.parsedOptions);
  }, r.prototype.toJSON = function(o) {
    var u = o ? !!o.keepComments : !1;
    return e.toObject([
      "type",
      this.type !== "rpc" && /* istanbul ignore next */
      this.type || void 0,
      "requestType",
      this.requestType,
      "requestStream",
      this.requestStream,
      "responseType",
      this.responseType,
      "responseStream",
      this.responseStream,
      "options",
      this.options,
      "comment",
      u ? this.comment : void 0,
      "parsedOptions",
      this.parsedOptions
    ]);
  }, r.prototype.resolve = function() {
    return this.resolved ? this : (this.resolvedRequestType = this.parent.lookupType(this.requestType), this.resolvedResponseType = this.parent.lookupType(this.responseType), t.prototype.resolve.call(this));
  }, method;
}
var service, hasRequiredService;
function requireService() {
  if (hasRequiredService) return service;
  hasRequiredService = 1, service = o;
  var t = requireNamespace();
  ((o.prototype = Object.create(t.prototype)).constructor = o).className = "Service";
  var e = requireMethod(), r = requireUtil(), s = rpc;
  function o(n, a) {
    t.call(this, n, a), this.methods = {}, this._methodsArray = null;
  }
  o.fromJSON = function(a, f) {
    var i = new o(a, f.options);
    if (f.methods)
      for (var l = Object.keys(f.methods), c = 0; c < l.length; ++c)
        i.add(e.fromJSON(l[c], f.methods[l[c]]));
    return f.nested && i.addJSON(f.nested), i.comment = f.comment, i;
  }, o.prototype.toJSON = function(a) {
    var f = t.prototype.toJSON.call(this, a), i = a ? !!a.keepComments : !1;
    return r.toObject([
      "options",
      f && f.options || void 0,
      "methods",
      t.arrayToJSON(this.methodsArray, a) || /* istanbul ignore next */
      {},
      "nested",
      f && f.nested || void 0,
      "comment",
      i ? this.comment : void 0
    ]);
  }, Object.defineProperty(o.prototype, "methodsArray", {
    get: function() {
      return this._methodsArray || (this._methodsArray = r.toArray(this.methods));
    }
  });
  function u(n) {
    return n._methodsArray = null, n;
  }
  return o.prototype.get = function(a) {
    return this.methods[a] || t.prototype.get.call(this, a);
  }, o.prototype.resolveAll = function() {
    for (var a = this.methodsArray, f = 0; f < a.length; ++f)
      a[f].resolve();
    return t.prototype.resolve.call(this);
  }, o.prototype.add = function(a) {
    if (this.get(a.name))
      throw Error("duplicate name '" + a.name + "' in " + this);
    return a instanceof e ? (this.methods[a.name] = a, a.parent = this, u(this)) : t.prototype.add.call(this, a);
  }, o.prototype.remove = function(a) {
    if (a instanceof e) {
      if (this.methods[a.name] !== a)
        throw Error(a + " is not a member of " + this);
      return delete this.methods[a.name], a.parent = null, u(this);
    }
    return t.prototype.remove.call(this, a);
  }, o.prototype.create = function(a, f, i) {
    for (var l = new s.Service(a, f, i), c = 0, h; c < /* initializes */
    this.methodsArray.length; ++c) {
      var d = r.lcFirst((h = this._methodsArray[c]).resolve().name).replace(/[^$\w_]/g, "");
      l[d] = r.codegen(["r", "c"], r.isReserved(d) ? d + "_" : d)("return this.rpcCall(m,q,s,r,c)")({
        m: h,
        q: h.resolvedRequestType.ctor,
        s: h.resolvedResponseType.ctor
      });
    }
    return l;
  }, service;
}
var message = Message, util = requireMinimal();
function Message(t) {
  if (t)
    for (var e = Object.keys(t), r = 0; r < e.length; ++r)
      this[e[r]] = t[e[r]];
}
Message.create = function t(e) {
  return this.$type.create(e);
};
Message.encode = function t(e, r) {
  return this.$type.encode(e, r);
};
Message.encodeDelimited = function t(e, r) {
  return this.$type.encodeDelimited(e, r);
};
Message.decode = function t(e) {
  return this.$type.decode(e);
};
Message.decodeDelimited = function t(e) {
  return this.$type.decodeDelimited(e);
};
Message.verify = function t(e) {
  return this.$type.verify(e);
};
Message.fromObject = function t(e) {
  return this.$type.fromObject(e);
};
Message.toObject = function t(e, r) {
  return this.$type.toObject(e, r);
};
Message.prototype.toJSON = function t() {
  return this.$type.toObject(this, util.toJSONOptions);
};
var decoder_1, hasRequiredDecoder;
function requireDecoder() {
  if (hasRequiredDecoder) return decoder_1;
  hasRequiredDecoder = 1, decoder_1 = o;
  var t = require_enum(), e = requireTypes(), r = requireUtil();
  function s(u) {
    return "missing required '" + u.name + "'";
  }
  function o(u) {
    var n = r.codegen(["r", "l"], u.name + "$decode")("if(!(r instanceof Reader))")("r=Reader.create(r)")("var c=l===undefined?r.len:r.pos+l,m=new this.ctor" + (u.fieldsArray.filter(function(h) {
      return h.map;
    }).length ? ",k,value" : ""))("while(r.pos<c){")("var t=r.uint32()");
    u.group && n("if((t&7)===4)")("break"), n("switch(t>>>3){");
    for (var a = 0; a < /* initializes */
    u.fieldsArray.length; ++a) {
      var f = u._fieldsArray[a].resolve(), i = f.resolvedType instanceof t ? "int32" : f.type, l = "m" + r.safeProp(f.name);
      n("case %i: {", f.id), f.map ? (n("if(%s===util.emptyObject)", l)("%s={}", l)("var c2 = r.uint32()+r.pos"), e.defaults[f.keyType] !== void 0 ? n("k=%j", e.defaults[f.keyType]) : n("k=null"), e.defaults[i] !== void 0 ? n("value=%j", e.defaults[i]) : n("value=null"), n("while(r.pos<c2){")("var tag2=r.uint32()")("switch(tag2>>>3){")("case 1: k=r.%s(); break", f.keyType)("case 2:"), e.basic[i] === void 0 ? n("value=types[%i].decode(r,r.uint32())", a) : n("value=r.%s()", i), n("break")("default:")("r.skipType(tag2&7)")("break")("}")("}"), e.long[f.keyType] !== void 0 ? n('%s[typeof k==="object"?util.longToHash(k):k]=value', l) : n("%s[k]=value", l)) : f.repeated ? (n("if(!(%s&&%s.length))", l, l)("%s=[]", l), e.packed[i] !== void 0 && n("if((t&7)===2){")("var c2=r.uint32()+r.pos")("while(r.pos<c2)")("%s.push(r.%s())", l, i)("}else"), e.basic[i] === void 0 ? n(f.resolvedType.group ? "%s.push(types[%i].decode(r))" : "%s.push(types[%i].decode(r,r.uint32()))", l, a) : n("%s.push(r.%s())", l, i)) : e.basic[i] === void 0 ? n(f.resolvedType.group ? "%s=types[%i].decode(r)" : "%s=types[%i].decode(r,r.uint32())", l, a) : n("%s=r.%s()", l, i), n("break")("}");
    }
    for (n("default:")("r.skipType(t&7)")("break")("}")("}"), a = 0; a < u._fieldsArray.length; ++a) {
      var c = u._fieldsArray[a];
      c.required && n("if(!m.hasOwnProperty(%j))", c.name)("throw util.ProtocolError(%j,{instance:m})", s(c));
    }
    return n("return m");
  }
  return decoder_1;
}
var verifier_1, hasRequiredVerifier;
function requireVerifier() {
  if (hasRequiredVerifier) return verifier_1;
  hasRequiredVerifier = 1, verifier_1 = u;
  var t = require_enum(), e = requireUtil();
  function r(n, a) {
    return n.name + ": " + a + (n.repeated && a !== "array" ? "[]" : n.map && a !== "object" ? "{k:" + n.keyType + "}" : "") + " expected";
  }
  function s(n, a, f, i) {
    if (a.resolvedType)
      if (a.resolvedType instanceof t) {
        n("switch(%s){", i)("default:")("return%j", r(a, "enum value"));
        for (var l = Object.keys(a.resolvedType.values), c = 0; c < l.length; ++c) n("case %i:", a.resolvedType.values[l[c]]);
        n("break")("}");
      } else
        n("{")("var e=types[%i].verify(%s);", f, i)("if(e)")("return%j+e", a.name + ".")("}");
    else
      switch (a.type) {
        case "int32":
        case "uint32":
        case "sint32":
        case "fixed32":
        case "sfixed32":
          n("if(!util.isInteger(%s))", i)("return%j", r(a, "integer"));
          break;
        case "int64":
        case "uint64":
        case "sint64":
        case "fixed64":
        case "sfixed64":
          n("if(!util.isInteger(%s)&&!(%s&&util.isInteger(%s.low)&&util.isInteger(%s.high)))", i, i, i, i)("return%j", r(a, "integer|Long"));
          break;
        case "float":
        case "double":
          n('if(typeof %s!=="number")', i)("return%j", r(a, "number"));
          break;
        case "bool":
          n('if(typeof %s!=="boolean")', i)("return%j", r(a, "boolean"));
          break;
        case "string":
          n("if(!util.isString(%s))", i)("return%j", r(a, "string"));
          break;
        case "bytes":
          n('if(!(%s&&typeof %s.length==="number"||util.isString(%s)))', i, i, i)("return%j", r(a, "buffer"));
          break;
      }
    return n;
  }
  function o(n, a, f) {
    switch (a.keyType) {
      case "int32":
      case "uint32":
      case "sint32":
      case "fixed32":
      case "sfixed32":
        n("if(!util.key32Re.test(%s))", f)("return%j", r(a, "integer key"));
        break;
      case "int64":
      case "uint64":
      case "sint64":
      case "fixed64":
      case "sfixed64":
        n("if(!util.key64Re.test(%s))", f)("return%j", r(a, "integer|Long key"));
        break;
      case "bool":
        n("if(!util.key2Re.test(%s))", f)("return%j", r(a, "boolean key"));
        break;
    }
    return n;
  }
  function u(n) {
    var a = e.codegen(["m"], n.name + "$verify")('if(typeof m!=="object"||m===null)')("return%j", "object expected"), f = n.oneofsArray, i = {};
    f.length && a("var p={}");
    for (var l = 0; l < /* initializes */
    n.fieldsArray.length; ++l) {
      var c = n._fieldsArray[l].resolve(), h = "m" + e.safeProp(c.name);
      if (c.optional && a("if(%s!=null&&m.hasOwnProperty(%j)){", h, c.name), c.map)
        a("if(!util.isObject(%s))", h)("return%j", r(c, "object"))("var k=Object.keys(%s)", h)("for(var i=0;i<k.length;++i){"), o(a, c, "k[i]"), s(a, c, l, h + "[k[i]]")("}");
      else if (c.repeated)
        a("if(!Array.isArray(%s))", h)("return%j", r(c, "array"))("for(var i=0;i<%s.length;++i){", h), s(a, c, l, h + "[i]")("}");
      else {
        if (c.partOf) {
          var d = e.safeProp(c.partOf.name);
          i[c.partOf.name] === 1 && a("if(p%s===1)", d)("return%j", c.partOf.name + ": multiple values"), i[c.partOf.name] = 1, a("p%s=1", d);
        }
        s(a, c, l, h);
      }
      c.optional && a("}");
    }
    return a("return null");
  }
  return verifier_1;
}
var converter = {}, hasRequiredConverter;
function requireConverter() {
  return hasRequiredConverter || (hasRequiredConverter = 1, function(t) {
    var e = t, r = require_enum(), s = requireUtil();
    function o(n, a, f, i) {
      var l = !1;
      if (a.resolvedType)
        if (a.resolvedType instanceof r) {
          n("switch(d%s){", i);
          for (var c = a.resolvedType.values, h = Object.keys(c), d = 0; d < h.length; ++d)
            c[h[d]] === a.typeDefault && !l && (n("default:")('if(typeof(d%s)==="number"){m%s=d%s;break}', i, i, i), a.repeated || n("break"), l = !0), n("case%j:", h[d])("case %i:", c[h[d]])("m%s=%j", i, c[h[d]])("break");
          n("}");
        } else n('if(typeof d%s!=="object")', i)("throw TypeError(%j)", a.fullName + ": object expected")("m%s=types[%i].fromObject(d%s)", i, f, i);
      else {
        var p = !1;
        switch (a.type) {
          case "double":
          case "float":
            n("m%s=Number(d%s)", i, i);
            break;
          case "uint32":
          case "fixed32":
            n("m%s=d%s>>>0", i, i);
            break;
          case "int32":
          case "sint32":
          case "sfixed32":
            n("m%s=d%s|0", i, i);
            break;
          case "uint64":
            p = !0;
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            n("if(util.Long)")("(m%s=util.Long.fromValue(d%s)).unsigned=%j", i, i, p)('else if(typeof d%s==="string")', i)("m%s=parseInt(d%s,10)", i, i)('else if(typeof d%s==="number")', i)("m%s=d%s", i, i)('else if(typeof d%s==="object")', i)("m%s=new util.LongBits(d%s.low>>>0,d%s.high>>>0).toNumber(%s)", i, i, i, p ? "true" : "");
            break;
          case "bytes":
            n('if(typeof d%s==="string")', i)("util.base64.decode(d%s,m%s=util.newBuffer(util.base64.length(d%s)),0)", i, i, i)("else if(d%s.length >= 0)", i)("m%s=d%s", i, i);
            break;
          case "string":
            n("m%s=String(d%s)", i, i);
            break;
          case "bool":
            n("m%s=Boolean(d%s)", i, i);
            break;
        }
      }
      return n;
    }
    e.fromObject = function(a) {
      var f = a.fieldsArray, i = s.codegen(["d"], a.name + "$fromObject")("if(d instanceof this.ctor)")("return d");
      if (!f.length) return i("return new this.ctor");
      i("var m=new this.ctor");
      for (var l = 0; l < f.length; ++l) {
        var c = f[l].resolve(), h = s.safeProp(c.name);
        c.map ? (i("if(d%s){", h)('if(typeof d%s!=="object")', h)("throw TypeError(%j)", c.fullName + ": object expected")("m%s={}", h)("for(var ks=Object.keys(d%s),i=0;i<ks.length;++i){", h), o(
          i,
          c,
          /* not sorted */
          l,
          h + "[ks[i]]"
        )("}")("}")) : c.repeated ? (i("if(d%s){", h)("if(!Array.isArray(d%s))", h)("throw TypeError(%j)", c.fullName + ": array expected")("m%s=[]", h)("for(var i=0;i<d%s.length;++i){", h), o(
          i,
          c,
          /* not sorted */
          l,
          h + "[i]"
        )("}")("}")) : (c.resolvedType instanceof r || i("if(d%s!=null){", h), o(
          i,
          c,
          /* not sorted */
          l,
          h
        ), c.resolvedType instanceof r || i("}"));
      }
      return i("return m");
    };
    function u(n, a, f, i) {
      if (a.resolvedType)
        a.resolvedType instanceof r ? n("d%s=o.enums===String?(types[%i].values[m%s]===undefined?m%s:types[%i].values[m%s]):m%s", i, f, i, i, f, i, i) : n("d%s=types[%i].toObject(m%s,o)", i, f, i);
      else {
        var l = !1;
        switch (a.type) {
          case "double":
          case "float":
            n("d%s=o.json&&!isFinite(m%s)?String(m%s):m%s", i, i, i, i);
            break;
          case "uint64":
            l = !0;
          case "int64":
          case "sint64":
          case "fixed64":
          case "sfixed64":
            n('if(typeof m%s==="number")', i)("d%s=o.longs===String?String(m%s):m%s", i, i, i)("else")("d%s=o.longs===String?util.Long.prototype.toString.call(m%s):o.longs===Number?new util.LongBits(m%s.low>>>0,m%s.high>>>0).toNumber(%s):m%s", i, i, i, i, l ? "true" : "", i);
            break;
          case "bytes":
            n("d%s=o.bytes===String?util.base64.encode(m%s,0,m%s.length):o.bytes===Array?Array.prototype.slice.call(m%s):m%s", i, i, i, i, i);
            break;
          default:
            n("d%s=m%s", i, i);
            break;
        }
      }
      return n;
    }
    e.toObject = function(a) {
      var f = a.fieldsArray.slice().sort(s.compareFieldsById);
      if (!f.length)
        return s.codegen()("return {}");
      for (var i = s.codegen(["m", "o"], a.name + "$toObject")("if(!o)")("o={}")("var d={}"), l = [], c = [], h = [], d = 0; d < f.length; ++d)
        f[d].partOf || (f[d].resolve().repeated ? l : f[d].map ? c : h).push(f[d]);
      if (l.length) {
        for (i("if(o.arrays||o.defaults){"), d = 0; d < l.length; ++d) i("d%s=[]", s.safeProp(l[d].name));
        i("}");
      }
      if (c.length) {
        for (i("if(o.objects||o.defaults){"), d = 0; d < c.length; ++d) i("d%s={}", s.safeProp(c[d].name));
        i("}");
      }
      if (h.length) {
        for (i("if(o.defaults){"), d = 0; d < h.length; ++d) {
          var p = h[d], v = s.safeProp(p.name);
          if (p.resolvedType instanceof r) i("d%s=o.enums===String?%j:%j", v, p.resolvedType.valuesById[p.typeDefault], p.typeDefault);
          else if (p.long) i("if(util.Long){")("var n=new util.Long(%i,%i,%j)", p.typeDefault.low, p.typeDefault.high, p.typeDefault.unsigned)("d%s=o.longs===String?n.toString():o.longs===Number?n.toNumber():n", v)("}else")("d%s=o.longs===String?%j:%i", v, p.typeDefault.toString(), p.typeDefault.toNumber());
          else if (p.bytes) {
            var g = "[" + Array.prototype.slice.call(p.typeDefault).join(",") + "]";
            i("if(o.bytes===String)d%s=%j", v, String.fromCharCode.apply(String, p.typeDefault))("else{")("d%s=%s", v, g)("if(o.bytes!==Array)d%s=util.newBuffer(d%s)", v, v)("}");
          } else i("d%s=%j", v, p.typeDefault);
        }
        i("}");
      }
      var m = !1;
      for (d = 0; d < f.length; ++d) {
        var p = f[d], _ = a._fieldsArray.indexOf(p), v = s.safeProp(p.name);
        p.map ? (m || (m = !0, i("var ks2")), i("if(m%s&&(ks2=Object.keys(m%s)).length){", v, v)("d%s={}", v)("for(var j=0;j<ks2.length;++j){"), u(
          i,
          p,
          /* sorted */
          _,
          v + "[ks2[j]]"
        )("}")) : p.repeated ? (i("if(m%s&&m%s.length){", v, v)("d%s=[]", v)("for(var j=0;j<m%s.length;++j){", v), u(
          i,
          p,
          /* sorted */
          _,
          v + "[j]"
        )("}")) : (i("if(m%s!=null&&m.hasOwnProperty(%j)){", v, p.name), u(
          i,
          p,
          /* sorted */
          _,
          v
        ), p.partOf && i("if(o.oneofs)")("d%s=%j", s.safeProp(p.partOf.name), p.name)), i("}");
      }
      return i("return d");
    };
  }(converter)), converter;
}
var wrappers = {};
(function(t) {
  var e = t, r = message;
  e[".google.protobuf.Any"] = {
    fromObject: function(s) {
      if (s && s["@type"]) {
        var o = s["@type"].substring(s["@type"].lastIndexOf("/") + 1), u = this.lookup(o);
        if (u) {
          var n = s["@type"].charAt(0) === "." ? s["@type"].slice(1) : s["@type"];
          return n.indexOf("/") === -1 && (n = "/" + n), this.create({
            type_url: n,
            value: u.encode(u.fromObject(s)).finish()
          });
        }
      }
      return this.fromObject(s);
    },
    toObject: function(s, o) {
      var u = "type.googleapis.com/", n = "", a = "";
      if (o && o.json && s.type_url && s.value) {
        a = s.type_url.substring(s.type_url.lastIndexOf("/") + 1), n = s.type_url.substring(0, s.type_url.lastIndexOf("/") + 1);
        var f = this.lookup(a);
        f && (s = f.decode(s.value));
      }
      if (!(s instanceof this.ctor) && s instanceof r) {
        var i = s.$type.toObject(s, o), l = s.$type.fullName[0] === "." ? s.$type.fullName.slice(1) : s.$type.fullName;
        return n === "" && (n = u), a = n + l, i["@type"] = a, i;
      }
      return this.toObject(s, o);
    }
  };
})(wrappers);
var type, hasRequiredType;
function requireType() {
  if (hasRequiredType) return type;
  hasRequiredType = 1, type = v;
  var t = requireNamespace();
  ((v.prototype = Object.create(t.prototype)).constructor = v).className = "Type";
  var e = require_enum(), r = requireOneof(), s = requireField(), o = requireMapfield(), u = requireService(), n = message, a = reader, f = writer, i = requireUtil(), l = requireEncoder(), c = requireDecoder(), h = requireVerifier(), d = requireConverter(), p = wrappers;
  function v(m, _) {
    t.call(this, m, _), this.fields = {}, this.oneofs = void 0, this.extensions = void 0, this.reserved = void 0, this.group = void 0, this._fieldsById = null, this._fieldsArray = null, this._oneofsArray = null, this._ctor = null;
  }
  Object.defineProperties(v.prototype, {
    /**
     * Message fields by id.
     * @name Type#fieldsById
     * @type {Object.<number,Field>}
     * @readonly
     */
    fieldsById: {
      get: function() {
        if (this._fieldsById)
          return this._fieldsById;
        this._fieldsById = {};
        for (var m = Object.keys(this.fields), _ = 0; _ < m.length; ++_) {
          var y = this.fields[m[_]], b = y.id;
          if (this._fieldsById[b])
            throw Error("duplicate id " + b + " in " + this);
          this._fieldsById[b] = y;
        }
        return this._fieldsById;
      }
    },
    /**
     * Fields of this message as an array for iteration.
     * @name Type#fieldsArray
     * @type {Field[]}
     * @readonly
     */
    fieldsArray: {
      get: function() {
        return this._fieldsArray || (this._fieldsArray = i.toArray(this.fields));
      }
    },
    /**
     * Oneofs of this message as an array for iteration.
     * @name Type#oneofsArray
     * @type {OneOf[]}
     * @readonly
     */
    oneofsArray: {
      get: function() {
        return this._oneofsArray || (this._oneofsArray = i.toArray(this.oneofs));
      }
    },
    /**
     * The registered constructor, if any registered, otherwise a generic constructor.
     * Assigning a function replaces the internal constructor. If the function does not extend {@link Message} yet, its prototype will be setup accordingly and static methods will be populated. If it already extends {@link Message}, it will just replace the internal constructor.
     * @name Type#ctor
     * @type {Constructor<{}>}
     */
    ctor: {
      get: function() {
        return this._ctor || (this.ctor = v.generateConstructor(this)());
      },
      set: function(m) {
        var _ = m.prototype;
        _ instanceof n || ((m.prototype = new n()).constructor = m, i.merge(m.prototype, _)), m.$type = m.prototype.$type = this, i.merge(m, n, !0), this._ctor = m;
        for (var y = 0; y < /* initializes */
        this.fieldsArray.length; ++y)
          this._fieldsArray[y].resolve();
        var b = {};
        for (y = 0; y < /* initializes */
        this.oneofsArray.length; ++y)
          b[this._oneofsArray[y].resolve().name] = {
            get: i.oneOfGetter(this._oneofsArray[y].oneof),
            set: i.oneOfSetter(this._oneofsArray[y].oneof)
          };
        y && Object.defineProperties(m.prototype, b);
      }
    }
  }), v.generateConstructor = function(_) {
    for (var y = i.codegen(["p"], _.name), b = 0, O; b < _.fieldsArray.length; ++b)
      (O = _._fieldsArray[b]).map ? y("this%s={}", i.safeProp(O.name)) : O.repeated && y("this%s=[]", i.safeProp(O.name));
    return y("if(p)for(var ks=Object.keys(p),i=0;i<ks.length;++i)if(p[ks[i]]!=null)")("this[ks[i]]=p[ks[i]]");
  };
  function g(m) {
    return m._fieldsById = m._fieldsArray = m._oneofsArray = null, delete m.encode, delete m.decode, delete m.verify, m;
  }
  return v.fromJSON = function(_, y) {
    var b = new v(_, y.options);
    b.extensions = y.extensions, b.reserved = y.reserved;
    for (var O = Object.keys(y.fields), E = 0; E < O.length; ++E)
      b.add(
        (typeof y.fields[O[E]].keyType < "u" ? o.fromJSON : s.fromJSON)(O[E], y.fields[O[E]])
      );
    if (y.oneofs)
      for (O = Object.keys(y.oneofs), E = 0; E < O.length; ++E)
        b.add(r.fromJSON(O[E], y.oneofs[O[E]]));
    if (y.nested)
      for (O = Object.keys(y.nested), E = 0; E < O.length; ++E) {
        var x = y.nested[O[E]];
        b.add(
          // most to least likely
          (x.id !== void 0 ? s.fromJSON : x.fields !== void 0 ? v.fromJSON : x.values !== void 0 ? e.fromJSON : x.methods !== void 0 ? u.fromJSON : t.fromJSON)(O[E], x)
        );
      }
    return y.extensions && y.extensions.length && (b.extensions = y.extensions), y.reserved && y.reserved.length && (b.reserved = y.reserved), y.group && (b.group = !0), y.comment && (b.comment = y.comment), b;
  }, v.prototype.toJSON = function(_) {
    var y = t.prototype.toJSON.call(this, _), b = _ ? !!_.keepComments : !1;
    return i.toObject([
      "options",
      y && y.options || void 0,
      "oneofs",
      t.arrayToJSON(this.oneofsArray, _),
      "fields",
      t.arrayToJSON(this.fieldsArray.filter(function(O) {
        return !O.declaringField;
      }), _) || {},
      "extensions",
      this.extensions && this.extensions.length ? this.extensions : void 0,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "group",
      this.group || void 0,
      "nested",
      y && y.nested || void 0,
      "comment",
      b ? this.comment : void 0
    ]);
  }, v.prototype.resolveAll = function() {
    for (var _ = this.fieldsArray, y = 0; y < _.length; )
      _[y++].resolve();
    var b = this.oneofsArray;
    for (y = 0; y < b.length; )
      b[y++].resolve();
    return t.prototype.resolveAll.call(this);
  }, v.prototype.get = function(_) {
    return this.fields[_] || this.oneofs && this.oneofs[_] || this.nested && this.nested[_] || null;
  }, v.prototype.add = function(_) {
    if (this.get(_.name))
      throw Error("duplicate name '" + _.name + "' in " + this);
    if (_ instanceof s && _.extend === void 0) {
      if (this._fieldsById ? (
        /* istanbul ignore next */
        this._fieldsById[_.id]
      ) : this.fieldsById[_.id])
        throw Error("duplicate id " + _.id + " in " + this);
      if (this.isReservedId(_.id))
        throw Error("id " + _.id + " is reserved in " + this);
      if (this.isReservedName(_.name))
        throw Error("name '" + _.name + "' is reserved in " + this);
      return _.parent && _.parent.remove(_), this.fields[_.name] = _, _.message = this, _.onAdd(this), g(this);
    }
    return _ instanceof r ? (this.oneofs || (this.oneofs = {}), this.oneofs[_.name] = _, _.onAdd(this), g(this)) : t.prototype.add.call(this, _);
  }, v.prototype.remove = function(_) {
    if (_ instanceof s && _.extend === void 0) {
      if (!this.fields || this.fields[_.name] !== _)
        throw Error(_ + " is not a member of " + this);
      return delete this.fields[_.name], _.parent = null, _.onRemove(this), g(this);
    }
    if (_ instanceof r) {
      if (!this.oneofs || this.oneofs[_.name] !== _)
        throw Error(_ + " is not a member of " + this);
      return delete this.oneofs[_.name], _.parent = null, _.onRemove(this), g(this);
    }
    return t.prototype.remove.call(this, _);
  }, v.prototype.isReservedId = function(_) {
    return t.isReservedId(this.reserved, _);
  }, v.prototype.isReservedName = function(_) {
    return t.isReservedName(this.reserved, _);
  }, v.prototype.create = function(_) {
    return new this.ctor(_);
  }, v.prototype.setup = function() {
    for (var _ = this.fullName, y = [], b = 0; b < /* initializes */
    this.fieldsArray.length; ++b)
      y.push(this._fieldsArray[b].resolve().resolvedType);
    this.encode = l(this)({
      Writer: f,
      types: y,
      util: i
    }), this.decode = c(this)({
      Reader: a,
      types: y,
      util: i
    }), this.verify = h(this)({
      types: y,
      util: i
    }), this.fromObject = d.fromObject(this)({
      types: y,
      util: i
    }), this.toObject = d.toObject(this)({
      types: y,
      util: i
    });
    var O = p[_];
    if (O) {
      var E = Object.create(this);
      E.fromObject = this.fromObject, this.fromObject = O.fromObject.bind(E), E.toObject = this.toObject, this.toObject = O.toObject.bind(E);
    }
    return this;
  }, v.prototype.encode = function(_, y) {
    return this.setup().encode(_, y);
  }, v.prototype.encodeDelimited = function(_, y) {
    return this.encode(_, y && y.len ? y.fork() : y).ldelim();
  }, v.prototype.decode = function(_, y) {
    return this.setup().decode(_, y);
  }, v.prototype.decodeDelimited = function(_) {
    return _ instanceof a || (_ = a.create(_)), this.decode(_, _.uint32());
  }, v.prototype.verify = function(_) {
    return this.setup().verify(_);
  }, v.prototype.fromObject = function(_) {
    return this.setup().fromObject(_);
  }, v.prototype.toObject = function(_, y) {
    return this.setup().toObject(_, y);
  }, v.d = function(_) {
    return function(b) {
      i.decorateType(b, _);
    };
  }, type;
}
var root, hasRequiredRoot;
function requireRoot() {
  if (hasRequiredRoot) return root;
  hasRequiredRoot = 1, root = f;
  var t = requireNamespace();
  ((f.prototype = Object.create(t.prototype)).constructor = f).className = "Root";
  var e = requireField(), r = require_enum(), s = requireOneof(), o = requireUtil(), u, n, a;
  function f(h) {
    t.call(this, "", h), this.deferred = [], this.files = [];
  }
  f.fromJSON = function(d, p) {
    return p || (p = new f()), d.options && p.setOptions(d.options), p.addJSON(d.nested);
  }, f.prototype.resolvePath = o.path.resolve, f.prototype.fetch = o.fetch;
  function i() {
  }
  f.prototype.load = function h(d, p, v) {
    typeof p == "function" && (v = p, p = void 0);
    var g = this;
    if (!v)
      return o.asPromise(h, g, d, p);
    var m = v === i;
    function _(R, q) {
      if (v) {
        if (m)
          throw R;
        var A = v;
        v = null, A(R, q);
      }
    }
    function y(R) {
      var q = R.lastIndexOf("google/protobuf/");
      if (q > -1) {
        var A = R.substring(q);
        if (A in a) return A;
      }
      return null;
    }
    function b(R, q) {
      try {
        if (o.isString(q) && q.charAt(0) === "{" && (q = JSON.parse(q)), !o.isString(q))
          g.setOptions(q.options).addJSON(q.nested);
        else {
          n.filename = R;
          var A = n(q, g, p), F, N = 0;
          if (A.imports)
            for (; N < A.imports.length; ++N)
              (F = y(A.imports[N]) || g.resolvePath(R, A.imports[N])) && O(F);
          if (A.weakImports)
            for (N = 0; N < A.weakImports.length; ++N)
              (F = y(A.weakImports[N]) || g.resolvePath(R, A.weakImports[N])) && O(F, !0);
        }
      } catch (J) {
        _(J);
      }
      !m && !E && _(null, g);
    }
    function O(R, q) {
      if (R = y(R) || R, !(g.files.indexOf(R) > -1)) {
        if (g.files.push(R), R in a) {
          m ? b(R, a[R]) : (++E, setTimeout(function() {
            --E, b(R, a[R]);
          }));
          return;
        }
        if (m) {
          var A;
          try {
            A = o.fs.readFileSync(R).toString("utf8");
          } catch (F) {
            q || _(F);
            return;
          }
          b(R, A);
        } else
          ++E, g.fetch(R, function(F, N) {
            if (--E, !!v) {
              if (F) {
                q ? E || _(null, g) : _(F);
                return;
              }
              b(R, N);
            }
          });
      }
    }
    var E = 0;
    o.isString(d) && (d = [d]);
    for (var x = 0, P; x < d.length; ++x)
      (P = g.resolvePath("", d[x])) && O(P);
    if (m)
      return g;
    E || _(null, g);
  }, f.prototype.loadSync = function(d, p) {
    if (!o.isNode)
      throw Error("not supported");
    return this.load(d, p, i);
  }, f.prototype.resolveAll = function() {
    if (this.deferred.length)
      throw Error("unresolvable extensions: " + this.deferred.map(function(d) {
        return "'extend " + d.extend + "' in " + d.parent.fullName;
      }).join(", "));
    return t.prototype.resolveAll.call(this);
  };
  var l = /^[A-Z]/;
  function c(h, d) {
    var p = d.parent.lookup(d.extend);
    if (p) {
      var v = new e(d.fullName, d.id, d.type, d.rule, void 0, d.options);
      return p.get(v.name) || (v.declaringField = d, d.extensionField = v, p.add(v)), !0;
    }
    return !1;
  }
  return f.prototype._handleAdd = function(d) {
    if (d instanceof e)
      /* an extension field (implies not part of a oneof) */
      d.extend !== void 0 && /* not already handled */
      !d.extensionField && (c(this, d) || this.deferred.push(d));
    else if (d instanceof r)
      l.test(d.name) && (d.parent[d.name] = d.values);
    else if (!(d instanceof s)) {
      if (d instanceof u)
        for (var p = 0; p < this.deferred.length; )
          c(this, this.deferred[p]) ? this.deferred.splice(p, 1) : ++p;
      for (var v = 0; v < /* initializes */
      d.nestedArray.length; ++v)
        this._handleAdd(d._nestedArray[v]);
      l.test(d.name) && (d.parent[d.name] = d);
    }
  }, f.prototype._handleRemove = function(d) {
    if (d instanceof e) {
      if (
        /* an extension field */
        d.extend !== void 0
      )
        if (
          /* already handled */
          d.extensionField
        )
          d.extensionField.parent.remove(d.extensionField), d.extensionField = null;
        else {
          var p = this.deferred.indexOf(d);
          p > -1 && this.deferred.splice(p, 1);
        }
    } else if (d instanceof r)
      l.test(d.name) && delete d.parent[d.name];
    else if (d instanceof t) {
      for (var v = 0; v < /* initializes */
      d.nestedArray.length; ++v)
        this._handleRemove(d._nestedArray[v]);
      l.test(d.name) && delete d.parent[d.name];
    }
  }, f._configure = function(h, d, p) {
    u = h, n = d, a = p;
  }, root;
}
var hasRequiredUtil;
function requireUtil() {
  if (hasRequiredUtil) return util$1.exports;
  hasRequiredUtil = 1;
  var t = util$1.exports = requireMinimal(), e = roots, r, s;
  t.codegen = codegen_1, t.fetch = fetch_1, t.path = path, t.fs = t.inquire("fs"), t.toArray = function(i) {
    if (i) {
      for (var l = Object.keys(i), c = new Array(l.length), h = 0; h < l.length; )
        c[h] = i[l[h++]];
      return c;
    }
    return [];
  }, t.toObject = function(i) {
    for (var l = {}, c = 0; c < i.length; ) {
      var h = i[c++], d = i[c++];
      d !== void 0 && (l[h] = d);
    }
    return l;
  };
  var o = /\\/g, u = /"/g;
  t.isReserved = function(i) {
    return /^(?:do|if|in|for|let|new|try|var|case|else|enum|eval|false|null|this|true|void|with|break|catch|class|const|super|throw|while|yield|delete|export|import|public|return|static|switch|typeof|default|extends|finally|package|private|continue|debugger|function|arguments|interface|protected|implements|instanceof)$/.test(i);
  }, t.safeProp = function(i) {
    return !/^[$\w_]+$/.test(i) || t.isReserved(i) ? '["' + i.replace(o, "\\\\").replace(u, '\\"') + '"]' : "." + i;
  }, t.ucFirst = function(i) {
    return i.charAt(0).toUpperCase() + i.substring(1);
  };
  var n = /_([a-z])/g;
  t.camelCase = function(i) {
    return i.substring(0, 1) + i.substring(1).replace(n, function(l, c) {
      return c.toUpperCase();
    });
  }, t.compareFieldsById = function(i, l) {
    return i.id - l.id;
  }, t.decorateType = function(i, l) {
    if (i.$type)
      return l && i.$type.name !== l && (t.decorateRoot.remove(i.$type), i.$type.name = l, t.decorateRoot.add(i.$type)), i.$type;
    r || (r = requireType());
    var c = new r(l || i.name);
    return t.decorateRoot.add(c), c.ctor = i, Object.defineProperty(i, "$type", { value: c, enumerable: !1 }), Object.defineProperty(i.prototype, "$type", { value: c, enumerable: !1 }), c;
  };
  var a = 0;
  return t.decorateEnum = function(i) {
    if (i.$type)
      return i.$type;
    s || (s = require_enum());
    var l = new s("Enum" + a++, i);
    return t.decorateRoot.add(l), Object.defineProperty(i, "$type", { value: l, enumerable: !1 }), l;
  }, t.setProperty = function(i, l, c) {
    function h(d, p, v) {
      var g = p.shift();
      if (g === "__proto__" || g === "prototype")
        return d;
      if (p.length > 0)
        d[g] = h(d[g] || {}, p, v);
      else {
        var m = d[g];
        m && (v = [].concat(m).concat(v)), d[g] = v;
      }
      return d;
    }
    if (typeof i != "object")
      throw TypeError("dst must be an object");
    if (!l)
      throw TypeError("path must be specified");
    return l = l.split("."), h(i, l, c);
  }, Object.defineProperty(t, "decorateRoot", {
    get: function() {
      return e.decorated || (e.decorated = new (requireRoot())());
    }
  }), util$1.exports;
}
var object, hasRequiredObject;
function requireObject() {
  if (hasRequiredObject) return object;
  hasRequiredObject = 1, object = r, r.className = "ReflectionObject";
  var t = requireUtil(), e;
  function r(s, o) {
    if (!t.isString(s))
      throw TypeError("name must be a string");
    if (o && !t.isObject(o))
      throw TypeError("options must be an object");
    this.options = o, this.parsedOptions = null, this.name = s, this.parent = null, this.resolved = !1, this.comment = null, this.filename = null;
  }
  return Object.defineProperties(r.prototype, {
    /**
     * Reference to the root namespace.
     * @name ReflectionObject#root
     * @type {Root}
     * @readonly
     */
    root: {
      get: function() {
        for (var s = this; s.parent !== null; )
          s = s.parent;
        return s;
      }
    },
    /**
     * Full name including leading dot.
     * @name ReflectionObject#fullName
     * @type {string}
     * @readonly
     */
    fullName: {
      get: function() {
        for (var s = [this.name], o = this.parent; o; )
          s.unshift(o.name), o = o.parent;
        return s.join(".");
      }
    }
  }), r.prototype.toJSON = /* istanbul ignore next */
  function() {
    throw Error();
  }, r.prototype.onAdd = function(o) {
    this.parent && this.parent !== o && this.parent.remove(this), this.parent = o, this.resolved = !1;
    var u = o.root;
    u instanceof e && u._handleAdd(this);
  }, r.prototype.onRemove = function(o) {
    var u = o.root;
    u instanceof e && u._handleRemove(this), this.parent = null, this.resolved = !1;
  }, r.prototype.resolve = function() {
    return this.resolved ? this : (this.root instanceof e && (this.resolved = !0), this);
  }, r.prototype.getOption = function(o) {
    if (this.options)
      return this.options[o];
  }, r.prototype.setOption = function(o, u, n) {
    return (!n || !this.options || this.options[o] === void 0) && ((this.options || (this.options = {}))[o] = u), this;
  }, r.prototype.setParsedOption = function(o, u, n) {
    this.parsedOptions || (this.parsedOptions = []);
    var a = this.parsedOptions;
    if (n) {
      var f = a.find(function(c) {
        return Object.prototype.hasOwnProperty.call(c, o);
      });
      if (f) {
        var i = f[o];
        t.setProperty(i, n, u);
      } else
        f = {}, f[o] = t.setProperty({}, n, u), a.push(f);
    } else {
      var l = {};
      l[o] = u, a.push(l);
    }
    return this;
  }, r.prototype.setOptions = function(o, u) {
    if (o)
      for (var n = Object.keys(o), a = 0; a < n.length; ++a)
        this.setOption(n[a], o[n[a]], u);
    return this;
  }, r.prototype.toString = function() {
    var o = this.constructor.className, u = this.fullName;
    return u.length ? o + " " + u : o;
  }, r._configure = function(s) {
    e = s;
  }, object;
}
var _enum, hasRequired_enum;
function require_enum() {
  if (hasRequired_enum) return _enum;
  hasRequired_enum = 1, _enum = s;
  var t = requireObject();
  ((s.prototype = Object.create(t.prototype)).constructor = s).className = "Enum";
  var e = requireNamespace(), r = requireUtil();
  function s(o, u, n, a, f, i) {
    if (t.call(this, o, n), u && typeof u != "object")
      throw TypeError("values must be an object");
    if (this.valuesById = {}, this.values = Object.create(this.valuesById), this.comment = a, this.comments = f || {}, this.valuesOptions = i, this.reserved = void 0, u)
      for (var l = Object.keys(u), c = 0; c < l.length; ++c)
        typeof u[l[c]] == "number" && (this.valuesById[this.values[l[c]] = u[l[c]]] = l[c]);
  }
  return s.fromJSON = function(u, n) {
    var a = new s(u, n.values, n.options, n.comment, n.comments);
    return a.reserved = n.reserved, a;
  }, s.prototype.toJSON = function(u) {
    var n = u ? !!u.keepComments : !1;
    return r.toObject([
      "options",
      this.options,
      "valuesOptions",
      this.valuesOptions,
      "values",
      this.values,
      "reserved",
      this.reserved && this.reserved.length ? this.reserved : void 0,
      "comment",
      n ? this.comment : void 0,
      "comments",
      n ? this.comments : void 0
    ]);
  }, s.prototype.add = function(u, n, a, f) {
    if (!r.isString(u))
      throw TypeError("name must be a string");
    if (!r.isInteger(n))
      throw TypeError("id must be an integer");
    if (this.values[u] !== void 0)
      throw Error("duplicate name '" + u + "' in " + this);
    if (this.isReservedId(n))
      throw Error("id " + n + " is reserved in " + this);
    if (this.isReservedName(u))
      throw Error("name '" + u + "' is reserved in " + this);
    if (this.valuesById[n] !== void 0) {
      if (!(this.options && this.options.allow_alias))
        throw Error("duplicate id " + n + " in " + this);
      this.values[u] = n;
    } else
      this.valuesById[this.values[u] = n] = u;
    return f && (this.valuesOptions === void 0 && (this.valuesOptions = {}), this.valuesOptions[u] = f || null), this.comments[u] = a || null, this;
  }, s.prototype.remove = function(u) {
    if (!r.isString(u))
      throw TypeError("name must be a string");
    var n = this.values[u];
    if (n == null)
      throw Error("name '" + u + "' does not exist in " + this);
    return delete this.valuesById[n], delete this.values[u], delete this.comments[u], this.valuesOptions && delete this.valuesOptions[u], this;
  }, s.prototype.isReservedId = function(u) {
    return e.isReservedId(this.reserved, u);
  }, s.prototype.isReservedName = function(u) {
    return e.isReservedName(this.reserved, u);
  }, _enum;
}
var encoder_1, hasRequiredEncoder;
function requireEncoder() {
  if (hasRequiredEncoder) return encoder_1;
  hasRequiredEncoder = 1, encoder_1 = o;
  var t = require_enum(), e = requireTypes(), r = requireUtil();
  function s(u, n, a, f) {
    return n.resolvedType.group ? u("types[%i].encode(%s,w.uint32(%i)).uint32(%i)", a, f, (n.id << 3 | 3) >>> 0, (n.id << 3 | 4) >>> 0) : u("types[%i].encode(%s,w.uint32(%i).fork()).ldelim()", a, f, (n.id << 3 | 2) >>> 0);
  }
  function o(u) {
    for (var n = r.codegen(["m", "w"], u.name + "$encode")("if(!w)")("w=Writer.create()"), a, f, i = (
      /* initializes */
      u.fieldsArray.slice().sort(r.compareFieldsById)
    ), a = 0; a < i.length; ++a) {
      var l = i[a].resolve(), c = u._fieldsArray.indexOf(l), h = l.resolvedType instanceof t ? "int32" : l.type, d = e.basic[h];
      f = "m" + r.safeProp(l.name), l.map ? (n("if(%s!=null&&Object.hasOwnProperty.call(m,%j)){", f, l.name)("for(var ks=Object.keys(%s),i=0;i<ks.length;++i){", f)("w.uint32(%i).fork().uint32(%i).%s(ks[i])", (l.id << 3 | 2) >>> 0, 8 | e.mapKey[l.keyType], l.keyType), d === void 0 ? n("types[%i].encode(%s[ks[i]],w.uint32(18).fork()).ldelim().ldelim()", c, f) : n(".uint32(%i).%s(%s[ks[i]]).ldelim()", 16 | d, h, f), n("}")("}")) : l.repeated ? (n("if(%s!=null&&%s.length){", f, f), l.packed && e.packed[h] !== void 0 ? n("w.uint32(%i).fork()", (l.id << 3 | 2) >>> 0)("for(var i=0;i<%s.length;++i)", f)("w.%s(%s[i])", h, f)("w.ldelim()") : (n("for(var i=0;i<%s.length;++i)", f), d === void 0 ? s(n, l, c, f + "[i]") : n("w.uint32(%i).%s(%s[i])", (l.id << 3 | d) >>> 0, h, f)), n("}")) : (l.optional && n("if(%s!=null&&Object.hasOwnProperty.call(m,%j))", f, l.name), d === void 0 ? s(n, l, c, f) : n("w.uint32(%i).%s(%s)", (l.id << 3 | d) >>> 0, h, f));
    }
    return n("return w");
  }
  return encoder_1;
}
var protobuf = indexLight.exports = indexMinimal;
protobuf.build = "light";
function load(t, e, r) {
  return typeof e == "function" ? (r = e, e = new protobuf.Root()) : e || (e = new protobuf.Root()), e.load(t, r);
}
protobuf.load = load;
function loadSync(t, e) {
  return e || (e = new protobuf.Root()), e.loadSync(t);
}
protobuf.loadSync = loadSync;
protobuf.encoder = requireEncoder();
protobuf.decoder = requireDecoder();
protobuf.verifier = requireVerifier();
protobuf.converter = requireConverter();
protobuf.ReflectionObject = requireObject();
protobuf.Namespace = requireNamespace();
protobuf.Root = requireRoot();
protobuf.Enum = require_enum();
protobuf.Type = requireType();
protobuf.Field = requireField();
protobuf.OneOf = requireOneof();
protobuf.MapField = requireMapfield();
protobuf.Service = requireService();
protobuf.Method = requireMethod();
protobuf.Message = message;
protobuf.wrappers = wrappers;
protobuf.types = requireTypes();
protobuf.util = requireUtil();
protobuf.ReflectionObject._configure(protobuf.Root);
protobuf.Namespace._configure(protobuf.Type, protobuf.Service, protobuf.Enum);
protobuf.Root._configure(protobuf.Type);
protobuf.Field._configure(protobuf.Type);
var indexLightExports = indexLight.exports, light = indexLightExports;
const protobufjs = /* @__PURE__ */ getDefaultExportFromCjs(light), proto_data = { options: { syntax: "proto3", optimize_for: "LITE_RUNTIME" }, nested: { nil: { nested: { xit: { nested: { proto: { nested: { MessageType: { values: { MessageType_FrameRequest: 0, MessageType_FrameResponse: 1, MessageType_FrameCache: 2, MessageType_ValueRequest: 3, MessageType_ValueResponse: 4, MessageType_ValueUpdate: 5, MessageType_SignalRequest: 6, MessageType_SignalResponse: 7, MessageType_SignalNotify: 8, MessageType_FileRequest: 9, MessageType_FileResponse: 10 } }, FrameRequest: { fields: { id: { type: "string", id: 1 } } }, FrameResponse: { oneofs: { value: { oneof: ["file", "content"] } }, fields: { id: { type: "string", id: 1 }, file: { type: "string", id: 2 }, content: { type: "string", id: 3 } } }, Value: { oneofs: { value: { oneof: ["value_boolean", "value_number", "value_double", "value_string", "value_buffer"] } }, fields: { id: { type: "string", id: 1 }, value_boolean: { type: "bool", id: 2 }, value_number: { type: "int64", id: 3 }, value_double: { type: "double", id: 4 }, value_string: { type: "string", id: 5 }, value_buffer: { type: "bytes", id: 6 } } }, ValueRequest: { oneofs: { _tag: { oneof: ["tag"] } }, fields: { id: { type: "string", id: 1 }, tag: { type: "string", id: 2, options: { proto3_optional: !0 } } } }, ValueResponse: { oneofs: { _tag: { oneof: ["tag"] } }, fields: { id: { type: "string", id: 1 }, tag: { type: "string", id: 2, options: { proto3_optional: !0 } }, values: { rule: "repeated", type: "Value", id: 3 } } }, ValueUpdate: { oneofs: { _tag: { oneof: ["tag"] } }, fields: { id: { type: "string", id: 1 }, tag: { type: "string", id: 2, options: { proto3_optional: !0 } }, value: { type: "Value", id: 3 } } }, SignalRequest: { fields: { id: { type: "string", id: 1 } } }, Signal: { oneofs: { _type: { oneof: ["type"] } }, fields: { id: { type: "string", id: 1 }, type: { type: "string", id: 2, options: { proto3_optional: !0 } } } }, SignalResponse: { oneofs: { _tag: { oneof: ["tag"] } }, fields: { id: { type: "string", id: 1 }, tag: { type: "string", id: 2, options: { proto3_optional: !0 } }, signals: { rule: "repeated", type: "Signal", id: 3 } } }, SignalNotify: { oneofs: { _tag: { oneof: ["tag"] }, arg: { oneof: ["arg_boolean", "arg_number", "arg_double", "arg_string", "arg_buffer"] } }, fields: { frame_id: { type: "string", id: 1 }, tag: { type: "string", id: 2, options: { proto3_optional: !0 } }, signal_id: { type: "string", id: 3 }, arg_boolean: { type: "bool", id: 4 }, arg_number: { type: "int64", id: 5 }, arg_double: { type: "double", id: 6 }, arg_string: { type: "string", id: 7 }, arg_buffer: { type: "bytes", id: 8 } } }, FileRequest: { fields: { target: { type: "string", id: 1 } } }, FileResponse: { fields: { target: { type: "string", id: 1 }, content: { type: "string", id: 2 }, metadata: { type: "bytes", id: 3 } } }, FileInfo: { fields: { target: { type: "string", id: 1 }, metadata: { type: "bytes", id: 2 } } }, FrameCache: { fields: { id: { type: "string", id: 1 }, content: { type: "string", id: 2 }, files: { rule: "repeated", type: "FileInfo", id: 3 } } } } } } } } } } }, nil_xit_proto = protobufjs.Root.fromJSON(proto_data).lookup(
  "nil.xit.proto"
), bundle = async (t) => new Promise((e, r) => {
  const s = new WorkerWrapper();
  s.postMessage({ type: "init", host: t.host, id: t.id }), s.addEventListener("message", async (o) => {
    if (o.data.ok) {
      if (o.data.files) {
        const u = concat([
          header(nil_xit_proto.MessageType.MessageType_FrameCache),
          nil_xit_proto.FrameCache.encode({
            id: t.id,
            content: o.data.code,
            files: o.data.files
          }).finish()
        ]);
        service_publish({ host: t.host }, u);
      }
      e(
        await import(
          /* @vite-ignore */
          "data:text/javascript;base64," + btoa(unescape(encodeURIComponent(o.data.code)))
        )
      );
    } else
      r(o.data.err);
    s.terminate();
  });
}), subscriber_queue = [];
function writable(t, e = noop$1) {
  let r = null;
  const s = /* @__PURE__ */ new Set();
  function o(a) {
    if (safe_not_equal(t, a) && (t = a, r)) {
      const f = !subscriber_queue.length;
      for (const i of s)
        i[1](), subscriber_queue.push(i, t);
      if (f) {
        for (let i = 0; i < subscriber_queue.length; i += 2)
          subscriber_queue[i][0](subscriber_queue[i + 1]);
        subscriber_queue.length = 0;
      }
    }
  }
  function u(a) {
    o(a(
      /** @type {T} */
      t
    ));
  }
  function n(a, f = noop$1) {
    const i = [a, f];
    return s.add(i), s.size === 1 && (r = e(o, u) || noop$1), a(
      /** @type {T} */
      t
    ), () => {
      s.delete(i), s.size === 0 && r && (r(), r = null);
    };
  }
  return { set: o, update: u, subscribe: n };
}
function get(t) {
  let e;
  return subscribe_to_store(t, (r) => e = r)(), e;
}
const make_values = async ({
  id: t,
  tag: e,
  host: r,
  service: s
}) => {
  const o = await service_fetch(
    { host: r },
    concat([
      header(nil_xit_proto.MessageType.MessageType_ValueRequest),
      nil_xit_proto.ValueRequest.encode({ id: t, tag: e }).finish()
    ]),
    (n, a) => {
      if (n === nil_xit_proto.MessageType.MessageType_ValueResponse) {
        const f = nil_xit_proto.ValueResponse.decode(a);
        if (f.id === t && (e == null || e === f.tag))
          return f.values;
      }
    }
  ), u = {
    value_boolean: /* @__PURE__ */ new Map(),
    value_double: /* @__PURE__ */ new Map(),
    value_number: /* @__PURE__ */ new Map(),
    value_string: /* @__PURE__ */ new Map(),
    value_buffer: /* @__PURE__ */ new Map()
  };
  for (const n of o) {
    const a = writable(n[n.value]);
    a.subscribe((f) => {
      const i = { id: n.id, [n.value]: f, value: n.value }, l = { id: t, tag: e, value: i }, c = concat([
        header(nil_xit_proto.MessageType.MessageType_ValueUpdate),
        nil_xit_proto.ValueUpdate.encode(l).finish()
      ]);
      s.publish(c);
    }), u[n.value].set(n.id, a);
  }
  return s.on_message((n, a) => {
    var l;
    const f = new DataView(a.buffer).getUint32(0, !1), i = a.slice(4);
    if (f === nil_xit_proto.MessageType.MessageType_ValueUpdate) {
      const c = nil_xit_proto.ValueUpdate.decode(i), h = c.value.value;
      u[h].get(c.value.id), (l = u[h].get(c.value.id)) == null || l.set(c.value[h]);
    }
  }), u;
}, make_signals = async ({
  id: t,
  tag: e,
  host: r,
  service: s
}) => {
  const o = await service_fetch(
    { host: r },
    concat([
      header(nil_xit_proto.MessageType.MessageType_SignalRequest),
      nil_xit_proto.SignalRequest.encode({ id: t }).finish()
    ]),
    (n, a) => {
      if (n === nil_xit_proto.MessageType.MessageType_SignalResponse) {
        const f = nil_xit_proto.SignalResponse.decode(a);
        if (f.id === t)
          return f.signals;
      }
    }
  ), u = {
    arg_none: /* @__PURE__ */ new Map(),
    arg_boolean: /* @__PURE__ */ new Map(),
    arg_double: /* @__PURE__ */ new Map(),
    arg_number: /* @__PURE__ */ new Map(),
    arg_string: /* @__PURE__ */ new Map(),
    arg_buffer: /* @__PURE__ */ new Map()
  };
  for (const { id: n, type: a } of o)
    a == null ? u.arg_none.set(n, () => {
      const f = { frame_id: t, signal_id: n, tag: e }, i = concat([
        header(nil_xit_proto.MessageType.MessageType_SignalNotify),
        nil_xit_proto.SignalNotify.encode(f).finish()
      ]);
      s.publish(i);
    }) : u[a].set(n, (f) => {
      const i = { frame_id: t, signal_id: n, tag: e, [a]: f }, l = concat([
        header(nil_xit_proto.MessageType.MessageType_SignalNotify),
        nil_xit_proto.SignalNotify.encode(i).finish()
      ]);
      s.publish(l);
    });
  return u;
}, create_context = (t, e, r) => ({
  values: {
    boolean: (s, o) => t.value_boolean.get(s) ?? writable(o),
    double: (s, o) => t.value_double.get(s) ?? writable(o),
    string: (s, o) => t.value_string.get(s) ?? writable(o),
    number: (s, o) => t.value_number.get(s) ?? writable(o),
    buffer: (s, o) => t.value_buffer.get(s) ?? writable(o),
    json: (s, o, u) => {
      const n = t.value_buffer.get(s) ?? writable(u.encode(o));
      let a = t.value_buffer.has(s) ? u.decode(get(n)) : o;
      return {
        set: (f) => {
          a = f, n.set(u.encode(f));
        },
        subscribe: (f) => n.subscribe((i) => f(a)),
        update: (f) => {
          n.update((i) => (a = f(i), u.encode(i)));
        }
      };
    }
  },
  signals: {
    none: (s) => e.arg_none.get(s) ?? (() => {
    }),
    boolean: (s) => e.arg_boolean.get(s) ?? ((o) => {
    }),
    double: (s) => e.arg_double.get(s) ?? ((o) => {
    }),
    number: (s) => e.arg_number.get(s) ?? ((o) => {
    }),
    string: (s) => e.arg_string.get(s) ?? ((o) => {
    }),
    buffer: (s) => e.arg_buffer.get(s) ?? ((o) => {
    }),
    json: (s, o) => {
      const u = e.arg_buffer.get(s) ?? ((n) => {
      });
      return (n) => u(o(n));
    }
  },
  loader: r
}), create_app = async (t, e, r, s) => {
  await test_connection({ host: t });
  const o = new Service$1({ host: t }), [u, n, { action: a }] = await Promise.all([
    make_values({ id: e, tag: r, host: t, service: o }),
    make_signals({ id: e, tag: r, host: t, service: o }),
    bundle({ host: t, id: e })
  ]);
  return (f) => {
    const i = /* @__PURE__ */ new Map();
    i.set("nil.xit", create_context(u, n, s));
    const { destroy: l } = a(f, i);
    return o.start(), {
      destroy: () => {
        o.stop(), l();
      }
    };
  };
};
var root_1 = /* @__PURE__ */ template('<div style="display: contents"></div>'), root_2 = /* @__PURE__ */ template("<div> </div>"), root_3 = /* @__PURE__ */ template("<div> </div>");
function Client(t, e) {
  push(e, !0);
  var r = comment(), s = first_child(r);
  await_block(
    s,
    () => create_app(e.host, e.frame, e.tag, e.loader),
    (o) => {
      var u = root_3(), n = child(u);
      template_effect(() => set_text(n, `Loading ${(e.tag ? `${e.frame}-${e.tag}` : e.frame) ?? ""}...`)), append(o, u);
    },
    (o, u) => {
      var n = root_1();
      action(n, (a) => get$1(u)(a)), append(o, n);
    },
    (o, u) => {
      var n = root_2(), a = child(n);
      template_effect(() => set_text(a, (console.log(get$1(u)), "Something went wrong..."))), append(o, n);
    }
  ), append(t, r), pop();
}
function Component(t, e) {
  push(e, !0);
  const r = (o) => {
    const u = {
      one: (n, a) => r([{ frame: n, tag: a }]),
      all: (n) => r(n)
    };
    return (n) => {
      n.style.display = "contents";
      const a = o.map(({ frame: f, tag: i }) => mount(Client, {
        target: n,
        props: {
          loader: u,
          host: e.data.server,
          frame: f,
          tag: i
        }
      }));
      return {
        destroy: () => a.forEach((f) => unmount(f))
      };
    };
  };
  var s = /* @__PURE__ */ derived(() => ({
    one: (o, u) => r([{ frame: o, tag: u }]),
    all: (o) => r(o)
  }));
  Client(t, {
    get loader() {
      return get$1(s);
    },
    get host() {
      return e.data.server;
    },
    get frame() {
      return e.data.frame;
    },
    get tag() {
      return e.data.tag;
    }
  }), pop();
}
const create_component = (t, e) => {
  const r = mount(Component, { target: t, props: { data: { ...e } } });
  return () => unmount(r);
};
export {
  create_component
};
