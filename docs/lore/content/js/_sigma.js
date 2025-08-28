// Version 2.4.0
var Sigma;
/******/ (() => { // webpackBootstrap
/******/    var __webpack_modules__ = ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
/**
 * Sigma.js Bundle Endpoint
 * ========================
 *
 * The library endpoint.
 * Will be built so that it exports a global `Sigma` class, that also exposes
 * useful classes as static properties.
 * @module
 */
var sigma_1 = __importDefault(__webpack_require__(1));
var camera_1 = __importDefault(__webpack_require__(3));
var quadtree_1 = __importDefault(__webpack_require__(14));
var mouse_1 = __importDefault(__webpack_require__(12));
var Sigma = /** @class */ (function (_super) {
   __extends(Sigma, _super);
   function Sigma() {
      return _super !== null && _super.apply(this, arguments) || this;
   }
   Sigma.Camera = camera_1.default;
   Sigma.QuadTree = quadtree_1.default;
   Sigma.MouseCaptor = mouse_1.default;
   Sigma.Sigma = sigma_1.default;
   return Sigma;
}(sigma_1.default));
module.exports = Sigma;


/***/ }),
/* 1 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __assign = (this && this.__assign) || function () {
   __assign = Object.assign || function(t) {
      for (var s, i = 1, n = arguments.length; i < n; i++) {
         s = arguments[i];
         for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
            t[p] = s[p];
      }
      return t;
   };
   return __assign.apply(this, arguments);
};
var __values = (this && this.__values) || function(o) {
   var s = typeof Symbol === "function" && Symbol.iterator, m = s && o[s], i = 0;
   if (m) return m.call(o);
   if (o && typeof o.length === "number") return {
      next: function () {
         if (o && i >= o.length) o = void 0;
         return { value: o && o[i++], done: !o };
      }
   };
   throw new TypeError(s ? "Object is not iterable." : "Symbol.iterator is not defined.");
};
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var extend_1 = __importDefault(__webpack_require__(2));
var camera_1 = __importDefault(__webpack_require__(3));
var mouse_1 = __importDefault(__webpack_require__(12));
var quadtree_1 = __importDefault(__webpack_require__(14));
var types_1 = __webpack_require__(10);
var utils_1 = __webpack_require__(5);
var labels_1 = __webpack_require__(15);
var settings_1 = __webpack_require__(16);
var touch_1 = __importDefault(__webpack_require__(36));
var matrices_1 = __webpack_require__(7);
var edge_collisions_1 = __webpack_require__(37);
/**
 * Constants.
 */
var X_LABEL_MARGIN = 150;
var Y_LABEL_MARGIN = 50;
/**
 * Important functions.
 */
function applyNodeDefaults(settings, key, data) {
   if (!data.hasOwnProperty("x") || !data.hasOwnProperty("y"))
      throw new Error("Sigma: could not find a valid position (x, y) for node \"".concat(key, "\". All your nodes must have a number \"x\" and \"y\". Maybe your forgot to apply a layout or your \"nodeReducer\" is not returning the correct data?"));
   if (!data.color)
      data.color = settings.defaultNodeColor;
   if (!data.label && data.label !== "")
      data.label = null;
   if (data.label !== undefined && data.label !== null)
      data.label = "" + data.label;
   else
      data.label = null;
   if (!data.size)
      data.size = 2;
   if (!data.hasOwnProperty("hidden"))
      data.hidden = false;
   if (!data.hasOwnProperty("highlighted"))
      data.highlighted = false;
   if (!data.hasOwnProperty("forceLabel"))
      data.forceLabel = false;
   if (!data.type || data.type === "")
      data.type = settings.defaultNodeType;
   if (!data.zIndex)
      data.zIndex = 0;
   return data;
}
function applyEdgeDefaults(settings, key, data) {
   if (!data.color)
      data.color = settings.defaultEdgeColor;
   if (!data.label)
      data.label = "";
   if (!data.size)
      data.size = 0.5;
   if (!data.hasOwnProperty("hidden"))
      data.hidden = false;
   if (!data.hasOwnProperty("forceLabel"))
      data.forceLabel = false;
   if (!data.type || data.type === "")
      data.type = settings.defaultEdgeType;
   if (!data.zIndex)
      data.zIndex = 0;
   return data;
}
/**
 * Main class.
 *
 * @constructor
 * @param {Graph}      graph    - Graph to render.
 * @param {HTMLElement} container - DOM container in which to render.
 * @param {object}     settings  - Optional settings.
 */
var Sigma = /** @class */ (function (_super) {
   __extends(Sigma, _super);
   function Sigma(graph, container, settings) {
      if (settings === void 0) { settings = {}; }
      var _this = _super.call(this) || this;
      _this.elements = {};
      _this.canvasContexts = {};
      _this.webGLContexts = {};
      _this.activeListeners = {};
      _this.quadtree = new quadtree_1.default();
      _this.labelGrid = new labels_1.LabelGrid();
      _this.nodeDataCache = {};
      _this.edgeDataCache = {};
      _this.nodesWithForcedLabels = [];
      _this.edgesWithForcedLabels = [];
      _this.nodeExtent = { x: [0, 1], y: [0, 1] };
      _this.matrix = (0, matrices_1.identity)();
      _this.invMatrix = (0, matrices_1.identity)();
      _this.correctionRatio = 1;
      _this.customBBox = null;
      _this.normalizationFunction = (0, utils_1.createNormalizationFunction)({
         x: [0, 1],
         y: [0, 1],
      });
      // Cache:
      _this.cameraSizeRatio = 1;
      // Starting dimensions and pixel ratio
      _this.width = 0;
      _this.height = 0;
      _this.pixelRatio = (0, utils_1.getPixelRatio)();
      // State
      _this.displayedLabels = new Set();
      _this.highlightedNodes = new Set();
      _this.hoveredNode = null;
      _this.hoveredEdge = null;
      _this.renderFrame = null;
      _this.renderHighlightedNodesFrame = null;
      _this.needToProcess = false;
      _this.needToSoftProcess = false;
      _this.checkEdgesEventsFrame = null;
      // Programs
      _this.nodePrograms = {};
      _this.nodeHoverPrograms = {};
      _this.edgePrograms = {};
      // Resolving settings
      _this.settings = (0, settings_1.resolveSettings)(settings);
      // Validating
      (0, settings_1.validateSettings)(_this.settings);
      (0, utils_1.validateGraph)(graph);
      if (!(container instanceof HTMLElement))
         throw new Error("Sigma: container should be an html element.");
      // Properties
      _this.graph = graph;
      _this.container = container;
      // Initializing contexts
      _this.createWebGLContext("edges", { preserveDrawingBuffer: true });
      _this.createCanvasContext("edgeLabels");
      _this.createWebGLContext("nodes");
      _this.createCanvasContext("labels");
      _this.createCanvasContext("hovers");
      _this.createWebGLContext("hoverNodes");
      _this.createCanvasContext("mouse");
      // Blending
      for (var key in _this.webGLContexts) {
         var gl = _this.webGLContexts[key];
         gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
         gl.enable(gl.BLEND);
      }
      // Loading programs
      for (var type in _this.settings.nodeProgramClasses) {
         var NodeProgramClass = _this.settings.nodeProgramClasses[type];
         _this.nodePrograms[type] = new NodeProgramClass(_this.webGLContexts.nodes, _this);
         var NodeHoverProgram = NodeProgramClass;
         if (type in _this.settings.nodeHoverProgramClasses) {
            NodeHoverProgram = _this.settings.nodeHoverProgramClasses[type];
         }
         _this.nodeHoverPrograms[type] = new NodeHoverProgram(_this.webGLContexts.hoverNodes, _this);
      }
      for (var type in _this.settings.edgeProgramClasses) {
         var EdgeProgramClass = _this.settings.edgeProgramClasses[type];
         _this.edgePrograms[type] = new EdgeProgramClass(_this.webGLContexts.edges, _this);
      }
      // Initial resize
      _this.resize();
      // Initializing the camera
      _this.camera = new camera_1.default();
      // Binding camera events
      _this.bindCameraHandlers();
      // Initializing captors
      _this.mouseCaptor = new mouse_1.default(_this.elements.mouse, _this);
      _this.touchCaptor = new touch_1.default(_this.elements.mouse, _this);
      // Binding event handlers
      _this.bindEventHandlers();
      // Binding graph handlers
      _this.bindGraphHandlers();
      // Trigger eventual settings-related things
      _this.handleSettingsUpdate();
      // Processing data for the first time & render
      _this.process();
      _this.render();
      return _this;
   }
   /**---------------------------------------------------------------------------
    * Internal methods.
    **---------------------------------------------------------------------------
    */
   /**
    * Internal function used to create a canvas element.
    * @param  {string} id - Context's id.
    * @return {Sigma}
    */
   Sigma.prototype.createCanvas = function (id) {
      var canvas = (0, utils_1.createElement)("canvas", {
         position: "absolute",
      }, {
         class: "sigma-".concat(id),
      });
      this.elements[id] = canvas;
      this.container.appendChild(canvas);
      return canvas;
   };
   /**
    * Internal function used to create a canvas context and add the relevant
    * DOM elements.
    *
    * @param  {string} id - Context's id.
    * @return {Sigma}
    */
   Sigma.prototype.createCanvasContext = function (id) {
      var canvas = this.createCanvas(id);
      var contextOptions = {
         preserveDrawingBuffer: false,
         antialias: false,
      };
      this.canvasContexts[id] = canvas.getContext("2d", contextOptions);
      return this;
   };
   /**
    * Internal function used to create a canvas context and add the relevant
    * DOM elements.
    *
    * @param  {string}  id     - Context's id.
    * @param  {object?} options - #getContext params to override (optional)
    * @return {Sigma}
    */
   Sigma.prototype.createWebGLContext = function (id, options) {
      var canvas = this.createCanvas(id);
      var contextOptions = __assign({ preserveDrawingBuffer: false, antialias: false }, (options || {}));
      var context;
      // First we try webgl2 for an easy performance boost
      context = canvas.getContext("webgl2", contextOptions);
      // Else we fall back to webgl
      if (!context)
         context = canvas.getContext("webgl", contextOptions);
      // Edge, I am looking right at you...
      if (!context)
         context = canvas.getContext("experimental-webgl", contextOptions);
      this.webGLContexts[id] = context;
      return this;
   };
   /**
    * Method binding camera handlers.
    *
    * @return {Sigma}
    */
   Sigma.prototype.bindCameraHandlers = function () {
      var _this = this;
      this.activeListeners.camera = function () {
         _this._scheduleRefresh();
      };
      this.camera.on("updated", this.activeListeners.camera);
      return this;
   };
   /**
    * Method that checks whether or not a node collides with a given position.
    */
   Sigma.prototype.mouseIsOnNode = function (_a, _b, size) {
      var x = _a.x, y = _a.y;
      var nodeX = _b.x, nodeY = _b.y;
      return (x > nodeX - size &&
         x < nodeX + size &&
         y > nodeY - size &&
         y < nodeY + size &&
         Math.sqrt(Math.pow(x - nodeX, 2) + Math.pow(y - nodeY, 2)) < size);
   };
   /**
    * Method that returns all nodes in quad at a given position.
    */
   Sigma.prototype.getQuadNodes = function (position) {
      var mouseGraphPosition = this.viewportToFramedGraph(position);
      return this.quadtree.point(mouseGraphPosition.x, 1 - mouseGraphPosition.y);
   };
   /**
    * Method that returns the closest node to a given position.
    */
   Sigma.prototype.getNodeAtPosition = function (position) {
      var x = position.x, y = position.y;
      var quadNodes = this.getQuadNodes(position);
      // We will hover the node whose center is closest to mouse
      var minDistance = Infinity, nodeAtPosition = null;
      for (var i = 0, l = quadNodes.length; i < l; i++) {
         var node = quadNodes[i];
         var data = this.nodeDataCache[node];
         var nodePosition = this.framedGraphToViewport(data);
         var size = this.scaleSize(data.size);
         if (!data.hidden && this.mouseIsOnNode(position, nodePosition, size)) {
            var distance = Math.sqrt(Math.pow(x - nodePosition.x, 2) + Math.pow(y - nodePosition.y, 2));
            // TODO: sort by min size also for cases where center is the same
            if (distance < minDistance) {
               minDistance = distance;
               nodeAtPosition = node;
            }
         }
      }
      return nodeAtPosition;
   };
   /**
    * Method binding event handlers.
    *
    * @return {Sigma}
    */
   Sigma.prototype.bindEventHandlers = function () {
      var _this = this;
      // Handling window resize
      this.activeListeners.handleResize = function () {
         _this.needToSoftProcess = true;
         _this._scheduleRefresh();
      };
      window.addEventListener("resize", this.activeListeners.handleResize);
      // Handling mouse move
      this.activeListeners.handleMove = function (e) {
         var baseEvent = {
            event: e,
            preventSigmaDefault: function () {
               e.preventSigmaDefault();
            },
         };
         var nodeToHover = _this.getNodeAtPosition(e);
         if (nodeToHover && _this.hoveredNode !== nodeToHover && !_this.nodeDataCache[nodeToHover].hidden) {
            // Handling passing from one node to the other directly
            if (_this.hoveredNode)
               _this.emit("leaveNode", __assign(__assign({}, baseEvent), { node: _this.hoveredNode }));
            _this.hoveredNode = nodeToHover;
            _this.emit("enterNode", __assign(__assign({}, baseEvent), { node: nodeToHover }));
            _this.scheduleHighlightedNodesRender();
            return;
         }
         // Checking if the hovered node is still hovered
         if (_this.hoveredNode) {
            var data = _this.nodeDataCache[_this.hoveredNode];
            var pos = _this.framedGraphToViewport(data);
            var size = _this.scaleSize(data.size);
            if (!_this.mouseIsOnNode(e, pos, size)) {
               var node = _this.hoveredNode;
               _this.hoveredNode = null;
               _this.emit("leaveNode", __assign(__assign({}, baseEvent), { node: node }));
               _this.scheduleHighlightedNodesRender();
               return;
            }
         }
         if (_this.settings.enableEdgeHoverEvents === true) {
            _this.checkEdgeHoverEvents(baseEvent);
         }
         else if (_this.settings.enableEdgeHoverEvents === "debounce") {
            if (!_this.checkEdgesEventsFrame)
               _this.checkEdgesEventsFrame = (0, utils_1.requestFrame)(function () {
                  _this.checkEdgeHoverEvents(baseEvent);
                  _this.checkEdgesEventsFrame = null;
               });
         }
      };
      // Handling click
      var createMouseListener = function (eventType) {
         return function (e) {
            var baseEvent = {
               event: e,
               preventSigmaDefault: function () {
                  e.preventSigmaDefault();
               },
            };
            var isFakeSigmaMouseEvent = e.original.isFakeSigmaMouseEvent;
            var nodeAtPosition = isFakeSigmaMouseEvent ? _this.getNodeAtPosition(e) : _this.hoveredNode;
            if (nodeAtPosition)
               return _this.emit("".concat(eventType, "Node"), __assign(__assign({}, baseEvent), { node: nodeAtPosition }));
            if (eventType === "wheel" ? _this.settings.enableEdgeWheelEvents : _this.settings.enableEdgeClickEvents) {
               var edge = _this.getEdgeAtPoint(e.x, e.y);
               if (edge)
                  return _this.emit("".concat(eventType, "Edge"), __assign(__assign({}, baseEvent), { edge: edge }));
            }
            return _this.emit("".concat(eventType, "Stage"), baseEvent);
         };
      };
      this.activeListeners.handleClick = createMouseListener("click");
      this.activeListeners.handleRightClick = createMouseListener("rightClick");
      this.activeListeners.handleDoubleClick = createMouseListener("doubleClick");
      this.activeListeners.handleWheel = createMouseListener("wheel");
      this.activeListeners.handleDown = createMouseListener("down");
      this.mouseCaptor.on("mousemove", this.activeListeners.handleMove);
      this.mouseCaptor.on("click", this.activeListeners.handleClick);
      this.mouseCaptor.on("rightClick", this.activeListeners.handleRightClick);
      this.mouseCaptor.on("doubleClick", this.activeListeners.handleDoubleClick);
      this.mouseCaptor.on("wheel", this.activeListeners.handleWheel);
      this.mouseCaptor.on("mousedown", this.activeListeners.handleDown);
      // TODO
      // Deal with Touch captor events
      return this;
   };
   /**
    * Method binding graph handlers
    *
    * @return {Sigma}
    */
   Sigma.prototype.bindGraphHandlers = function () {
      var _this = this;
      var graph = this.graph;
      this.activeListeners.graphUpdate = function () {
         _this.needToProcess = true;
         _this._scheduleRefresh();
      };
      this.activeListeners.softGraphUpdate = function () {
         _this.needToSoftProcess = true;
         _this._scheduleRefresh();
      };
      this.activeListeners.dropNodeGraphUpdate = function (e) {
         delete _this.nodeDataCache[e.key];
         if (_this.hoveredNode === e.key)
            _this.hoveredNode = null;
         _this.activeListeners.graphUpdate();
      };
      this.activeListeners.dropEdgeGraphUpdate = function (e) {
         delete _this.edgeDataCache[e.key];
         if (_this.hoveredEdge === e.key)
            _this.hoveredEdge = null;
         _this.activeListeners.graphUpdate();
      };
      this.activeListeners.clearEdgesGraphUpdate = function () {
         _this.edgeDataCache = {};
         _this.hoveredEdge = null;
         _this.activeListeners.graphUpdate();
      };
      this.activeListeners.clearGraphUpdate = function () {
         _this.nodeDataCache = {};
         _this.hoveredNode = null;
         _this.activeListeners.clearEdgesGraphUpdate();
      };
      graph.on("nodeAdded", this.activeListeners.graphUpdate);
      graph.on("nodeDropped", this.activeListeners.dropNodeGraphUpdate);
      graph.on("nodeAttributesUpdated", this.activeListeners.softGraphUpdate);
      graph.on("eachNodeAttributesUpdated", this.activeListeners.graphUpdate);
      graph.on("edgeAdded", this.activeListeners.graphUpdate);
      graph.on("edgeDropped", this.activeListeners.dropEdgeGraphUpdate);
      graph.on("edgeAttributesUpdated", this.activeListeners.softGraphUpdate);
      graph.on("eachEdgeAttributesUpdated", this.activeListeners.graphUpdate);
      graph.on("edgesCleared", this.activeListeners.clearEdgesGraphUpdate);
      graph.on("cleared", this.activeListeners.clearGraphUpdate);
      return this;
   };
   /**
    * Method used to unbind handlers from the graph.
    *
    * @return {undefined}
    */
   Sigma.prototype.unbindGraphHandlers = function () {
      var graph = this.graph;
      graph.removeListener("nodeAdded", this.activeListeners.graphUpdate);
      graph.removeListener("nodeDropped", this.activeListeners.dropNodeGraphUpdate);
      graph.removeListener("nodeAttributesUpdated", this.activeListeners.softGraphUpdate);
      graph.removeListener("eachNodeAttributesUpdated", this.activeListeners.graphUpdate);
      graph.removeListener("edgeAdded", this.activeListeners.graphUpdate);
      graph.removeListener("edgeDropped", this.activeListeners.dropEdgeGraphUpdate);
      graph.removeListener("edgeAttributesUpdated", this.activeListeners.softGraphUpdate);
      graph.removeListener("eachEdgeAttributesUpdated", this.activeListeners.graphUpdate);
      graph.removeListener("edgesCleared", this.activeListeners.clearEdgesGraphUpdate);
      graph.removeListener("cleared", this.activeListeners.clearGraphUpdate);
   };
   /**
    * Method dealing with "leaveEdge" and "enterEdge" events.
    *
    * @return {Sigma}
    */
   Sigma.prototype.checkEdgeHoverEvents = function (payload) {
      var edgeToHover = this.hoveredNode ? null : this.getEdgeAtPoint(payload.event.x, payload.event.y);
      if (edgeToHover !== this.hoveredEdge) {
         if (this.hoveredEdge)
            this.emit("leaveEdge", __assign(__assign({}, payload), { edge: this.hoveredEdge }));
         if (edgeToHover)
            this.emit("enterEdge", __assign(__assign({}, payload), { edge: edgeToHover }));
         this.hoveredEdge = edgeToHover;
      }
      return this;
   };
   /**
    * Method looking for an edge colliding with a given point at (x, y). Returns
    * the key of the edge if any, or null else.
    */
   Sigma.prototype.getEdgeAtPoint = function (x, y) {
      var e_1, _a;
      var _this = this;
      var _b = this, edgeDataCache = _b.edgeDataCache, nodeDataCache = _b.nodeDataCache;
      // Check first that pixel is colored:
      // Note that mouse positions must be corrected by pixel ratio to correctly
      // index the drawing buffer.
      if (!(0, edge_collisions_1.isPixelColored)(this.webGLContexts.edges, x * this.pixelRatio, y * this.pixelRatio))
         return null;
      // Check for each edge if it collides with the point:
      var _c = this.viewportToGraph({ x: x, y: y }), graphX = _c.x, graphY = _c.y;
      // To translate edge thicknesses to the graph system, we observe by how much
      // the length of a non-null edge is transformed to between the graph system
      // and the viewport system:
      var transformationRatio = 0;
      this.graph.someEdge(function (key, _, sourceId, targetId, _a, _b) {
         var xs = _a.x, ys = _a.y;
         var xt = _b.x, yt = _b.y;
         if (edgeDataCache[key].hidden || nodeDataCache[sourceId].hidden || nodeDataCache[targetId].hidden)
            return false;
         if (xs !== xt || ys !== yt) {
            var graphLength = Math.sqrt(Math.pow(xt - xs, 2) + Math.pow(yt - ys, 2));
            var _c = _this.graphToViewport({ x: xs, y: ys }), vp_xs = _c.x, vp_ys = _c.y;
            var _d = _this.graphToViewport({ x: xt, y: yt }), vp_xt = _d.x, vp_yt = _d.y;
            var viewportLength = Math.sqrt(Math.pow(vp_xt - vp_xs, 2) + Math.pow(vp_yt - vp_ys, 2));
            transformationRatio = graphLength / viewportLength;
            return true;
         }
      });
      // If no non-null edge has been found, return null:
      if (!transformationRatio)
         return null;
      // Now we can look for matching edges:
      var edges = this.graph.filterEdges(function (key, edgeAttributes, sourceId, targetId, sourcePosition, targetPosition) {
         if (edgeDataCache[key].hidden || nodeDataCache[sourceId].hidden || nodeDataCache[targetId].hidden)
            return false;
         if ((0, edge_collisions_1.doEdgeCollideWithPoint)(graphX, graphY, sourcePosition.x, sourcePosition.y, targetPosition.x, targetPosition.y,
         // Adapt the edge size to the zoom ratio:
         (edgeDataCache[key].size * transformationRatio) / _this.cameraSizeRatio)) {
            return true;
         }
      });
      if (edges.length === 0)
         return null; // no edges found
      // if none of the edges have a zIndex, selected the most recently created one to match the rendering order
      var selectedEdge = edges[edges.length - 1];
      // otherwise select edge with highest zIndex
      var highestZIndex = -Infinity;
      try {
         for (var edges_1 = __values(edges), edges_1_1 = edges_1.next(); !edges_1_1.done; edges_1_1 = edges_1.next()) {
            var edge = edges_1_1.value;
            var zIndex = this.graph.getEdgeAttribute(edge, "zIndex");
            if (zIndex >= highestZIndex) {
               selectedEdge = edge;
               highestZIndex = zIndex;
            }
         }
      }
      catch (e_1_1) { e_1 = { error: e_1_1 }; }
      finally {
         try {
            if (edges_1_1 && !edges_1_1.done && (_a = edges_1.return)) _a.call(edges_1);
         }
         finally { if (e_1) throw e_1.error; }
      }
      return selectedEdge;
   };
   /**
    * Method used to process the whole graph's data.
    *
    * @return {Sigma}
    */
   Sigma.prototype.process = function (keepArrays) {
      var _this = this;
      if (keepArrays === void 0) { keepArrays = false; }
      var graph = this.graph;
      var settings = this.settings;
      var dimensions = this.getDimensions();
      var nodeZExtent = [Infinity, -Infinity];
      var edgeZExtent = [Infinity, -Infinity];
      // Clearing the quad
      this.quadtree.clear();
      // Resetting the label grid
      // TODO: it's probably better to do this explicitly or on resizes for layout and anims
      this.labelGrid.resizeAndClear(dimensions, settings.labelGridCellSize);
      // Clear the highlightedNodes
      this.highlightedNodes = new Set();
      // Computing extents
      this.nodeExtent = (0, utils_1.graphExtent)(graph);
      // Resetting `forceLabel` indices
      this.nodesWithForcedLabels = [];
      this.edgesWithForcedLabels = [];
      // NOTE: it is important to compute this matrix after computing the node's extent
      // because #.getGraphDimensions relies on it
      var nullCamera = new camera_1.default();
      var nullCameraMatrix = (0, utils_1.matrixFromCamera)(nullCamera.getState(), this.getDimensions(), this.getGraphDimensions(), this.getSetting("stagePadding") || 0);
      // Rescaling function
      this.normalizationFunction = (0, utils_1.createNormalizationFunction)(this.customBBox || this.nodeExtent);
      var nodesPerPrograms = {};
      var nodes = graph.nodes();
      for (var i = 0, l = nodes.length; i < l; i++) {
         var node = nodes[i];
         // Node display data resolution:
         //   1. First we get the node's attributes
         //   2. We optionally reduce them using the function provided by the user
         //     Note that this function must return a total object and won't be merged
         //   3. We apply our defaults, while running some vital checks
         //   4. We apply the normalization function
         // We shallow copy node data to avoid dangerous behaviors from reducers
         var attr = Object.assign({}, graph.getNodeAttributes(node));
         if (settings.nodeReducer)
            attr = settings.nodeReducer(node, attr);
         var data = applyNodeDefaults(this.settings, node, attr);
         nodesPerPrograms[data.type] = (nodesPerPrograms[data.type] || 0) + 1;
         this.nodeDataCache[node] = data;
         this.normalizationFunction.applyTo(data);
         if (data.forceLabel)
            this.nodesWithForcedLabels.push(node);
         if (this.settings.zIndex) {
            if (data.zIndex < nodeZExtent[0])
               nodeZExtent[0] = data.zIndex;
            if (data.zIndex > nodeZExtent[1])
               nodeZExtent[1] = data.zIndex;
         }
      }
      for (var type in this.nodePrograms) {
         if (!this.nodePrograms.hasOwnProperty(type)) {
            throw new Error("Sigma: could not find a suitable program for node type \"".concat(type, "\"!"));
         }
         if (!keepArrays)
            this.nodePrograms[type].allocate(nodesPerPrograms[type] || 0);
         // We reset that count here, so that we can reuse it while calling the Program#process methods:
         nodesPerPrograms[type] = 0;
      }
      // Handling node z-index
      // TODO: z-index needs us to compute display data before hand
      if (this.settings.zIndex && nodeZExtent[0] !== nodeZExtent[1])
         nodes = (0, utils_1.zIndexOrdering)(nodeZExtent, function (node) { return _this.nodeDataCache[node].zIndex; }, nodes);
      for (var i = 0, l = nodes.length; i < l; i++) {
         var node = nodes[i];
         var data = this.nodeDataCache[node];
         this.quadtree.add(node, data.x, 1 - data.y, data.size / this.width);
         if (typeof data.label === "string" && !data.hidden)
            this.labelGrid.add(node, data.size, this.framedGraphToViewport(data, { matrix: nullCameraMatrix }));
         var nodeProgram = this.nodePrograms[data.type];
         if (!nodeProgram)
            throw new Error("Sigma: could not find a suitable program for node type \"".concat(data.type, "\"!"));
         nodeProgram.process(data, data.hidden, nodesPerPrograms[data.type]++);
         // Save the node in the highlighted set if needed
         if (data.highlighted && !data.hidden)
            this.highlightedNodes.add(node);
      }
      this.labelGrid.organize();
      var edgesPerPrograms = {};
      var edges = graph.edges();
      for (var i = 0, l = edges.length; i < l; i++) {
         var edge = edges[i];
         // Edge display data resolution:
         //   1. First we get the edge's attributes
         //   2. We optionally reduce them using the function provided by the user
         //     Note that this function must return a total object and won't be merged
         //   3. We apply our defaults, while running some vital checks
         // We shallow copy edge data to avoid dangerous behaviors from reducers
         var attr = Object.assign({}, graph.getEdgeAttributes(edge));
         if (settings.edgeReducer)
            attr = settings.edgeReducer(edge, attr);
         var data = applyEdgeDefaults(this.settings, edge, attr);
         edgesPerPrograms[data.type] = (edgesPerPrograms[data.type] || 0) + 1;
         this.edgeDataCache[edge] = data;
         if (data.forceLabel && !data.hidden)
            this.edgesWithForcedLabels.push(edge);
         if (this.settings.zIndex) {
            if (data.zIndex < edgeZExtent[0])
               edgeZExtent[0] = data.zIndex;
            if (data.zIndex > edgeZExtent[1])
               edgeZExtent[1] = data.zIndex;
         }
      }
      for (var type in this.edgePrograms) {
         if (!this.edgePrograms.hasOwnProperty(type)) {
            throw new Error("Sigma: could not find a suitable program for edge type \"".concat(type, "\"!"));
         }
         if (!keepArrays)
            this.edgePrograms[type].allocate(edgesPerPrograms[type] || 0);
         // We reset that count here, so that we can reuse it while calling the Program#process methods:
         edgesPerPrograms[type] = 0;
      }
      // Handling edge z-index
      if (this.settings.zIndex && edgeZExtent[0] !== edgeZExtent[1])
         edges = (0, utils_1.zIndexOrdering)(edgeZExtent, function (edge) { return _this.edgeDataCache[edge].zIndex; }, edges);
      for (var i = 0, l = edges.length; i < l; i++) {
         var edge = edges[i];
         var data = this.edgeDataCache[edge];
         var extremities = graph.extremities(edge), sourceData = this.nodeDataCache[extremities[0]], targetData = this.nodeDataCache[extremities[1]];
         var hidden = data.hidden || sourceData.hidden || targetData.hidden;
         this.edgePrograms[data.type].process(sourceData, targetData, data, hidden, edgesPerPrograms[data.type]++);
      }
      for (var type in this.edgePrograms) {
         var program = this.edgePrograms[type];
         if (!keepArrays && typeof program.computeIndices === "function")
            program.computeIndices();
      }
      return this;
   };
   /**
    * Method that backports potential settings updates where it's needed.
    * @private
    */
   Sigma.prototype.handleSettingsUpdate = function () {
      this.camera.minRatio = this.settings.minCameraRatio;
      this.camera.maxRatio = this.settings.maxCameraRatio;
      this.camera.setState(this.camera.validateState(this.camera.getState()));
      return this;
   };
   /**
    * Method that decides whether to reprocess graph or not, and then render the
    * graph.
    *
    * @return {Sigma}
    */
   Sigma.prototype._refresh = function () {
      // Do we need to process data?
      if (this.needToProcess) {
         this.process();
      }
      else if (this.needToSoftProcess) {
         this.process(true);
      }
      // Resetting state
      this.needToProcess = false;
      this.needToSoftProcess = false;
      // Rendering
      this.render();
      return this;
   };
   /**
    * Method that schedules a `_refresh` call if none has been scheduled yet. It
    * will then be processed next available frame.
    *
    * @return {Sigma}
    */
   Sigma.prototype._scheduleRefresh = function () {
      var _this = this;
      if (!this.renderFrame) {
         this.renderFrame = (0, utils_1.requestFrame)(function () {
            _this._refresh();
            _this.renderFrame = null;
         });
      }
      return this;
   };
   /**
    * Method used to render labels.
    *
    * @return {Sigma}
    */
   Sigma.prototype.renderLabels = function () {
      if (!this.settings.renderLabels)
         return this;
      var cameraState = this.camera.getState();
      // Selecting labels to draw
      var labelsToDisplay = this.labelGrid.getLabelsToDisplay(cameraState.ratio, this.settings.labelDensity);
      (0, extend_1.default)(labelsToDisplay, this.nodesWithForcedLabels);
      this.displayedLabels = new Set();
      // Drawing labels
      var context = this.canvasContexts.labels;
      for (var i = 0, l = labelsToDisplay.length; i < l; i++) {
         var node = labelsToDisplay[i];
         var data = this.nodeDataCache[node];
         // If the node was already drawn (like if it is eligible AND has
         // `forceLabel`), we don't want to draw it again
         // NOTE: we can do better probably
         if (this.displayedLabels.has(node))
            continue;
         // If the node is hidden, we don't need to display its label obviously
         if (data.hidden)
            continue;
         var _a = this.framedGraphToViewport(data), x = _a.x, y = _a.y;
         // NOTE: we can cache the labels we need to render until the camera's ratio changes
         var size = this.scaleSize(data.size);
         // Is node big enough?
         if (!data.forceLabel && size < this.settings.labelRenderedSizeThreshold)
            continue;
         // Is node actually on screen (with some margin)
         // NOTE: we used to rely on the quadtree for this, but the coordinates
         // conversion make it unreliable and at that point we already converted
         // to viewport coordinates and since the label grid already culls the
         // number of potential labels to display this looks like a good
         // performance compromise.
         // NOTE: labelGrid.getLabelsToDisplay could probably optimize by not
         // considering cells obviously outside of the range of the current
         // view rectangle.
         if (x < -X_LABEL_MARGIN ||
            x > this.width + X_LABEL_MARGIN ||
            y < -Y_LABEL_MARGIN ||
            y > this.height + Y_LABEL_MARGIN)
            continue;
         // Because displayed edge labels depend directly on actually rendered node
         // labels, we need to only add to this.displayedLabels nodes whose label
         // is rendered.
         // This makes this.displayedLabels depend on viewport, which might become
         // an issue once we start memoizing getLabelsToDisplay.
         this.displayedLabels.add(node);
         this.settings.labelRenderer(context, __assign(__assign({ key: node }, data), { size: size, x: x, y: y }), this.settings);
      }
      return this;
   };
   /**
    * Method used to render edge labels, based on which node labels were
    * rendered.
    *
    * @return {Sigma}
    */
   Sigma.prototype.renderEdgeLabels = function () {
      if (!this.settings.renderEdgeLabels)
         return this;
      var context = this.canvasContexts.edgeLabels;
      // Clearing
      context.clearRect(0, 0, this.width, this.height);
      var edgeLabelsToDisplay = (0, labels_1.edgeLabelsToDisplayFromNodes)({
         graph: this.graph,
         hoveredNode: this.hoveredNode,
         displayedNodeLabels: this.displayedLabels,
         highlightedNodes: this.highlightedNodes,
      }).concat(this.edgesWithForcedLabels);
      var displayedLabels = new Set();
      for (var i = 0, l = edgeLabelsToDisplay.length; i < l; i++) {
         var edge = edgeLabelsToDisplay[i], extremities = this.graph.extremities(edge), sourceData = this.nodeDataCache[extremities[0]], targetData = this.nodeDataCache[extremities[1]], edgeData = this.edgeDataCache[edge];
         // If the edge was already drawn (like if it is eligible AND has
         // `forceLabel`), we don't want to draw it again
         if (displayedLabels.has(edge))
            continue;
         // If the edge is hidden we don't need to display its label
         // NOTE: the test on sourceData & targetData is probably paranoid at this point?
         if (edgeData.hidden || sourceData.hidden || targetData.hidden) {
            continue;
         }
         this.settings.edgeLabelRenderer(context, __assign(__assign({ key: edge }, edgeData), { size: this.scaleSize(edgeData.size) }), __assign(__assign(__assign({ key: extremities[0] }, sourceData), this.framedGraphToViewport(sourceData)), { size: this.scaleSize(sourceData.size) }), __assign(__assign(__assign({ key: extremities[1] }, targetData), this.framedGraphToViewport(targetData)), { size: this.scaleSize(targetData.size) }), this.settings);
         displayedLabels.add(edge);
      }
      return this;
   };
   /**
    * Method used to render the highlighted nodes.
    *
    * @return {Sigma}
    */
   Sigma.prototype.renderHighlightedNodes = function () {
      var _this = this;
      var context = this.canvasContexts.hovers;
      // Clearing
      context.clearRect(0, 0, this.width, this.height);
      // Rendering
      var render = function (node) {
         var data = _this.nodeDataCache[node];
         var _a = _this.framedGraphToViewport(data), x = _a.x, y = _a.y;
         var size = _this.scaleSize(data.size);
         _this.settings.hoverRenderer(context, __assign(__assign({ key: node }, data), { size: size, x: x, y: y }), _this.settings);
      };
      var nodesToRender = [];
      if (this.hoveredNode && !this.nodeDataCache[this.hoveredNode].hidden) {
         nodesToRender.push(this.hoveredNode);
      }
      this.highlightedNodes.forEach(function (node) {
         // The hovered node has already been highlighted
         if (node !== _this.hoveredNode)
            nodesToRender.push(node);
      });
      // Draw labels:
      nodesToRender.forEach(function (node) { return render(node); });
      // Draw WebGL nodes on top of the labels:
      var nodesPerPrograms = {};
      // 1. Count nodes per type:
      nodesToRender.forEach(function (node) {
         var type = _this.nodeDataCache[node].type;
         nodesPerPrograms[type] = (nodesPerPrograms[type] || 0) + 1;
      });
      // 2. Allocate for each type for the proper number of nodes
      for (var type in this.nodeHoverPrograms) {
         this.nodeHoverPrograms[type].allocate(nodesPerPrograms[type] || 0);
         // Also reset count, to use when rendering:
         nodesPerPrograms[type] = 0;
      }
      // 3. Process all nodes to render:
      nodesToRender.forEach(function (node) {
         var data = _this.nodeDataCache[node];
         _this.nodeHoverPrograms[data.type].process(data, data.hidden, nodesPerPrograms[data.type]++);
      });
      // 4. Clear hovered nodes layer:
      this.webGLContexts.hoverNodes.clear(this.webGLContexts.hoverNodes.COLOR_BUFFER_BIT);
      // 5. Render:
      for (var type in this.nodeHoverPrograms) {
         var program = this.nodeHoverPrograms[type];
         program.bind();
         program.bufferData();
         program.render({
            matrix: this.matrix,
            width: this.width,
            height: this.height,
            ratio: this.camera.ratio,
            correctionRatio: this.correctionRatio / this.camera.ratio,
            scalingRatio: this.pixelRatio,
         });
      }
   };
   /**
    * Method used to schedule a hover render.
    *
    */
   Sigma.prototype.scheduleHighlightedNodesRender = function () {
      var _this = this;
      if (this.renderHighlightedNodesFrame || this.renderFrame)
         return;
      this.renderHighlightedNodesFrame = (0, utils_1.requestFrame)(function () {
         // Resetting state
         _this.renderHighlightedNodesFrame = null;
         // Rendering
         _this.renderHighlightedNodes();
         _this.renderEdgeLabels();
      });
   };
   /**
    * Method used to render.
    *
    * @return {Sigma}
    */
   Sigma.prototype.render = function () {
      var _this = this;
      this.emit("beforeRender");
      var exitRender = function () {
         _this.emit("afterRender");
         return _this;
      };
      // If a render was scheduled, we cancel it
      if (this.renderFrame) {
         (0, utils_1.cancelFrame)(this.renderFrame);
         this.renderFrame = null;
         this.needToProcess = false;
         this.needToSoftProcess = false;
      }
      // First we need to resize
      this.resize();
      // Clearing the canvases
      this.clear();
      // Recomputing useful camera-related values:
      this.updateCachedValues();
      // If we have no nodes we can stop right there
      if (!this.graph.order)
         return exitRender();
      // TODO: improve this heuristic or move to the captor itself?
      // TODO: deal with the touch captor here as well
      var mouseCaptor = this.mouseCaptor;
      var moving = this.camera.isAnimated() ||
         mouseCaptor.isMoving ||
         mouseCaptor.draggedEvents ||
         mouseCaptor.currentWheelDirection;
      // Then we need to extract a matrix from the camera
      var cameraState = this.camera.getState();
      var viewportDimensions = this.getDimensions();
      var graphDimensions = this.getGraphDimensions();
      var padding = this.getSetting("stagePadding") || 0;
      this.matrix = (0, utils_1.matrixFromCamera)(cameraState, viewportDimensions, graphDimensions, padding);
      this.invMatrix = (0, utils_1.matrixFromCamera)(cameraState, viewportDimensions, graphDimensions, padding, true);
      this.correctionRatio = (0, utils_1.getMatrixImpact)(this.matrix, cameraState, viewportDimensions);
      // Drawing nodes
      for (var type in this.nodePrograms) {
         var program = this.nodePrograms[type];
         program.bind();
         program.bufferData();
         program.render({
            matrix: this.matrix,
            width: this.width,
            height: this.height,
            ratio: cameraState.ratio,
            correctionRatio: this.correctionRatio / cameraState.ratio,
            scalingRatio: this.pixelRatio,
         });
      }
      // Drawing edges
      if (!this.settings.hideEdgesOnMove || !moving) {
         for (var type in this.edgePrograms) {
            var program = this.edgePrograms[type];
            program.bind();
            program.bufferData();
            program.render({
               matrix: this.matrix,
               width: this.width,
               height: this.height,
               ratio: cameraState.ratio,
               correctionRatio: this.correctionRatio / cameraState.ratio,
               scalingRatio: this.pixelRatio,
            });
         }
      }
      // Do not display labels on move per setting
      if (this.settings.hideLabelsOnMove && moving)
         return exitRender();
      this.renderLabels();
      this.renderEdgeLabels();
      this.renderHighlightedNodes();
      return exitRender();
   };
   /**
    * Internal method used to update expensive and therefore cached values
    * each time the camera state is updated.
    */
   Sigma.prototype.updateCachedValues = function () {
      var ratio = this.camera.getState().ratio;
      this.cameraSizeRatio = Math.sqrt(ratio);
   };
   /**---------------------------------------------------------------------------
    * Public API.
    **---------------------------------------------------------------------------
    */
   /**
    * Method returning the renderer's camera.
    *
    * @return {Camera}
    */
   Sigma.prototype.getCamera = function () {
      return this.camera;
   };
   /**
    * Method returning the container DOM element.
    *
    * @return {HTMLElement}
    */
   Sigma.prototype.getContainer = function () {
      return this.container;
   };
   /**
    * Method returning the renderer's graph.
    *
    * @return {Graph}
    */
   Sigma.prototype.getGraph = function () {
      return this.graph;
   };
   /**
    * Method used to set the renderer's graph.
    *
    * @return {Graph}
    */
   Sigma.prototype.setGraph = function (graph) {
      if (graph === this.graph)
         return;
      // Unbinding handlers on the current graph
      this.unbindGraphHandlers();
      // Clearing the graph data caches
      this.nodeDataCache = {};
      this.edgeDataCache = {};
      // Cleaning renderer state tied to the current graph
      this.displayedLabels.clear();
      this.highlightedNodes.clear();
      this.hoveredNode = null;
      this.hoveredEdge = null;
      this.nodesWithForcedLabels.length = 0;
      this.edgesWithForcedLabels.length = 0;
      if (this.checkEdgesEventsFrame !== null) {
         (0, utils_1.cancelFrame)(this.checkEdgesEventsFrame);
         this.checkEdgesEventsFrame = null;
      }
      // Installing new graph
      this.graph = graph;
      // Binding new handlers
      this.bindGraphHandlers();
      // Re-rendering now to avoid discrepancies from now to next frame
      this.process();
      this.render();
   };
   /**
    * Method returning the mouse captor.
    *
    * @return {MouseCaptor}
    */
   Sigma.prototype.getMouseCaptor = function () {
      return this.mouseCaptor;
   };
   /**
    * Method returning the touch captor.
    *
    * @return {TouchCaptor}
    */
   Sigma.prototype.getTouchCaptor = function () {
      return this.touchCaptor;
   };
   /**
    * Method returning the current renderer's dimensions.
    *
    * @return {Dimensions}
    */
   Sigma.prototype.getDimensions = function () {
      return { width: this.width, height: this.height };
   };
   /**
    * Method returning the current graph's dimensions.
    *
    * @return {Dimensions}
    */
   Sigma.prototype.getGraphDimensions = function () {
      var extent = this.customBBox || this.nodeExtent;
      return {
         width: extent.x[1] - extent.x[0] || 1,
         height: extent.y[1] - extent.y[0] || 1,
      };
   };
   /**
    * Method used to get all the sigma node attributes.
    * It's usefull for example to get the position of a node
    * and to get values that are set by the nodeReducer
    *
    * @param  {string} key - The node's key.
    * @return {NodeDisplayData | undefined} A copy of the desired node's attribute or undefined if not found
    */
   Sigma.prototype.getNodeDisplayData = function (key) {
      var node = this.nodeDataCache[key];
      return node ? Object.assign({}, node) : undefined;
   };
   /**
    * Method used to get all the sigma edge attributes.
    * It's usefull for example to get values that are set by the edgeReducer.
    *
    * @param  {string} key - The edge's key.
    * @return {EdgeDisplayData | undefined} A copy of the desired edge's attribute or undefined if not found
    */
   Sigma.prototype.getEdgeDisplayData = function (key) {
      var edge = this.edgeDataCache[key];
      return edge ? Object.assign({}, edge) : undefined;
   };
   /**
    * Method returning a copy of the settings collection.
    *
    * @return {Settings} A copy of the settings collection.
    */
   Sigma.prototype.getSettings = function () {
      return __assign({}, this.settings);
   };
   /**
    * Method returning the current value for a given setting key.
    *
    * @param  {string} key - The setting key to get.
    * @return {any} The value attached to this setting key or undefined if not found
    */
   Sigma.prototype.getSetting = function (key) {
      return this.settings[key];
   };
   /**
    * Method setting the value of a given setting key. Note that this will schedule
    * a new render next frame.
    *
    * @param  {string} key - The setting key to set.
    * @param  {any}   value - The value to set.
    * @return {Sigma}
    */
   Sigma.prototype.setSetting = function (key, value) {
      this.settings[key] = value;
      (0, settings_1.validateSettings)(this.settings);
      this.handleSettingsUpdate();
      this.needToProcess = true; // TODO: some keys may work with only needToSoftProcess or even nothing
      this._scheduleRefresh();
      return this;
   };
   /**
    * Method updating the value of a given setting key using the provided function.
    * Note that this will schedule a new render next frame.
    *
    * @param  {string}   key    - The setting key to set.
    * @param  {function} updater - The update function.
    * @return {Sigma}
    */
   Sigma.prototype.updateSetting = function (key, updater) {
      this.settings[key] = updater(this.settings[key]);
      (0, settings_1.validateSettings)(this.settings);
      this.handleSettingsUpdate();
      this.needToProcess = true; // TODO: some keys may work with only needToSoftProcess or even nothing
      this._scheduleRefresh();
      return this;
   };
   /**
    * Method used to resize the renderer.
    *
    * @return {Sigma}
    */
   Sigma.prototype.resize = function () {
      var previousWidth = this.width, previousHeight = this.height;
      this.width = this.container.offsetWidth;
      this.height = this.container.offsetHeight;
      this.pixelRatio = (0, utils_1.getPixelRatio)();
      if (this.width === 0) {
         if (this.settings.allowInvalidContainer)
            this.width = 1;
         else
            throw new Error("Sigma: Container has no width. You can set the allowInvalidContainer setting to true to stop seeing this error.");
      }
      if (this.height === 0) {
         if (this.settings.allowInvalidContainer)
            this.height = 1;
         else
            throw new Error("Sigma: Container has no height. You can set the allowInvalidContainer setting to true to stop seeing this error.");
      }
      // If nothing has changed, we can stop right here
      if (previousWidth === this.width && previousHeight === this.height)
         return this;
      this.emit("resize");
      // Sizing dom elements
      for (var id in this.elements) {
         var element = this.elements[id];
         element.style.width = this.width + "px";
         element.style.height = this.height + "px";
      }
      // Sizing canvas contexts
      for (var id in this.canvasContexts) {
         this.elements[id].setAttribute("width", this.width * this.pixelRatio + "px");
         this.elements[id].setAttribute("height", this.height * this.pixelRatio + "px");
         if (this.pixelRatio !== 1)
            this.canvasContexts[id].scale(this.pixelRatio, this.pixelRatio);
      }
      // Sizing WebGL contexts
      for (var id in this.webGLContexts) {
         this.elements[id].setAttribute("width", this.width * this.pixelRatio + "px");
         this.elements[id].setAttribute("height", this.height * this.pixelRatio + "px");
         this.webGLContexts[id].viewport(0, 0, this.width * this.pixelRatio, this.height * this.pixelRatio);
      }
      return this;
   };
   /**
    * Method used to clear all the canvases.
    *
    * @return {Sigma}
    */
   Sigma.prototype.clear = function () {
      this.webGLContexts.nodes.clear(this.webGLContexts.nodes.COLOR_BUFFER_BIT);
      this.webGLContexts.edges.clear(this.webGLContexts.edges.COLOR_BUFFER_BIT);
      this.webGLContexts.hoverNodes.clear(this.webGLContexts.hoverNodes.COLOR_BUFFER_BIT);
      this.canvasContexts.labels.clearRect(0, 0, this.width, this.height);
      this.canvasContexts.hovers.clearRect(0, 0, this.width, this.height);
      this.canvasContexts.edgeLabels.clearRect(0, 0, this.width, this.height);
      return this;
   };
   /**
    * Method used to refresh all computed data.
    *
    * @return {Sigma}
    */
   Sigma.prototype.refresh = function () {
      this.needToProcess = true;
      this._refresh();
      return this;
   };
   /**
    * Method used to refresh all computed data, at the next available frame.
    * If this method has already been called this frame, then it will only render once at the next available frame.
    *
    * @return {Sigma}
    */
   Sigma.prototype.scheduleRefresh = function () {
      this.needToProcess = true;
      this._scheduleRefresh();
      return this;
   };
   /**
    * Method used to (un)zoom, while preserving the position of a viewport point.
    * Used for instance to zoom "on the mouse cursor".
    *
    * @param viewportTarget
    * @param newRatio
    * @return {CameraState}
    */
   Sigma.prototype.getViewportZoomedState = function (viewportTarget, newRatio) {
      var _a = this.camera.getState(), ratio = _a.ratio, angle = _a.angle, x = _a.x, y = _a.y;
      // TODO: handle max zoom
      var ratioDiff = newRatio / ratio;
      var center = {
         x: this.width / 2,
         y: this.height / 2,
      };
      var graphMousePosition = this.viewportToFramedGraph(viewportTarget);
      var graphCenterPosition = this.viewportToFramedGraph(center);
      return {
         angle: angle,
         x: (graphMousePosition.x - graphCenterPosition.x) * (1 - ratioDiff) + x,
         y: (graphMousePosition.y - graphCenterPosition.y) * (1 - ratioDiff) + y,
         ratio: newRatio,
      };
   };
   /**
    * Method returning the abstract rectangle containing the graph according
    * to the camera's state.
    *
    * @return {object} - The view's rectangle.
    */
   Sigma.prototype.viewRectangle = function () {
      // TODO: reduce relative margin?
      var marginX = (0 * this.width) / 8, marginY = (0 * this.height) / 8;
      var p1 = this.viewportToFramedGraph({ x: 0 - marginX, y: 0 - marginY }), p2 = this.viewportToFramedGraph({ x: this.width + marginX, y: 0 - marginY }), h = this.viewportToFramedGraph({ x: 0, y: this.height + marginY });
      return {
         x1: p1.x,
         y1: p1.y,
         x2: p2.x,
         y2: p2.y,
         height: p2.y - h.y,
      };
   };
   /**
    * Method returning the coordinates of a point from the framed graph system to the viewport system. It allows
    * overriding anything that is used to get the translation matrix, or even the matrix itself.
    *
    * Be careful if overriding dimensions, padding or cameraState, as the computation of the matrix is not the lightest
    * of computations.
    */
   Sigma.prototype.framedGraphToViewport = function (coordinates, override) {
      if (override === void 0) { override = {}; }
      var recomputeMatrix = !!override.cameraState || !!override.viewportDimensions || !!override.graphDimensions;
      var matrix = override.matrix
         ? override.matrix
         : recomputeMatrix
            ? (0, utils_1.matrixFromCamera)(override.cameraState || this.camera.getState(), override.viewportDimensions || this.getDimensions(), override.graphDimensions || this.getGraphDimensions(), override.padding || this.getSetting("stagePadding") || 0)
            : this.matrix;
      var viewportPos = (0, matrices_1.multiplyVec2)(matrix, coordinates);
      return {
         x: ((1 + viewportPos.x) * this.width) / 2,
         y: ((1 - viewportPos.y) * this.height) / 2,
      };
   };
   /**
    * Method returning the coordinates of a point from the viewport system to the framed graph system. It allows
    * overriding anything that is used to get the translation matrix, or even the matrix itself.
    *
    * Be careful if overriding dimensions, padding or cameraState, as the computation of the matrix is not the lightest
    * of computations.
    */
   Sigma.prototype.viewportToFramedGraph = function (coordinates, override) {
      if (override === void 0) { override = {}; }
      var recomputeMatrix = !!override.cameraState || !!override.viewportDimensions || !override.graphDimensions;
      var invMatrix = override.matrix
         ? override.matrix
         : recomputeMatrix
            ? (0, utils_1.matrixFromCamera)(override.cameraState || this.camera.getState(), override.viewportDimensions || this.getDimensions(), override.graphDimensions || this.getGraphDimensions(), override.padding || this.getSetting("stagePadding") || 0, true)
            : this.invMatrix;
      var res = (0, matrices_1.multiplyVec2)(invMatrix, {
         x: (coordinates.x / this.width) * 2 - 1,
         y: 1 - (coordinates.y / this.height) * 2,
      });
      if (isNaN(res.x))
         res.x = 0;
      if (isNaN(res.y))
         res.y = 0;
      return res;
   };
   /**
    * Method used to translate a point's coordinates from the viewport system (pixel distance from the top-left of the
    * stage) to the graph system (the reference system of data as they are in the given graph instance).
    *
    * This method accepts an optional camera which can be useful if you need to translate coordinates
    * based on a different view than the one being currently being displayed on screen.
    *
    * @param {Coordinates}              viewportPoint
    * @param {CoordinateConversionOverride} override
    */
   Sigma.prototype.viewportToGraph = function (viewportPoint, override) {
      if (override === void 0) { override = {}; }
      return this.normalizationFunction.inverse(this.viewportToFramedGraph(viewportPoint, override));
   };
   /**
    * Method used to translate a point's coordinates from the graph system (the reference system of data as they are in
    * the given graph instance) to the viewport system (pixel distance from the top-left of the stage).
    *
    * This method accepts an optional camera which can be useful if you need to translate coordinates
    * based on a different view than the one being currently being displayed on screen.
    *
    * @param {Coordinates}              graphPoint
    * @param {CoordinateConversionOverride} override
    */
   Sigma.prototype.graphToViewport = function (graphPoint, override) {
      if (override === void 0) { override = {}; }
      return this.framedGraphToViewport(this.normalizationFunction(graphPoint), override);
   };
   /**
    * Method returning the graph's bounding box.
    *
    * @return {{ x: Extent, y: Extent }}
    */
   Sigma.prototype.getBBox = function () {
      return (0, utils_1.graphExtent)(this.graph);
   };
   /**
    * Method returning the graph's custom bounding box, if any.
    *
    * @return {{ x: Extent, y: Extent } | null}
    */
   Sigma.prototype.getCustomBBox = function () {
      return this.customBBox;
   };
   /**
    * Method used to override the graph's bounding box with a custom one. Give `null` as the argument to stop overriding.
    *
    * @return {Sigma}
    */
   Sigma.prototype.setCustomBBox = function (customBBox) {
      this.customBBox = customBBox;
      this._scheduleRefresh();
      return this;
   };
   /**
    * Method used to shut the container & release event listeners.
    *
    * @return {undefined}
    */
   Sigma.prototype.kill = function () {
      // Emitting "kill" events so that plugins and such can cleanup
      this.emit("kill");
      // Releasing events
      this.removeAllListeners();
      // Releasing camera handlers
      this.camera.removeListener("updated", this.activeListeners.camera);
      // Releasing DOM events & captors
      window.removeEventListener("resize", this.activeListeners.handleResize);
      this.mouseCaptor.kill();
      this.touchCaptor.kill();
      // Releasing graph handlers
      this.unbindGraphHandlers();
      // Releasing cache & state
      this.quadtree = new quadtree_1.default();
      this.nodeDataCache = {};
      this.edgeDataCache = {};
      this.nodesWithForcedLabels = [];
      this.edgesWithForcedLabels = [];
      this.highlightedNodes.clear();
      // Clearing frames
      if (this.renderFrame) {
         (0, utils_1.cancelFrame)(this.renderFrame);
         this.renderFrame = null;
      }
      if (this.renderHighlightedNodesFrame) {
         (0, utils_1.cancelFrame)(this.renderHighlightedNodesFrame);
         this.renderHighlightedNodesFrame = null;
      }
      // Destroying canvases
      var container = this.container;
      while (container.firstChild)
         container.removeChild(container.firstChild);
   };
   /**
    * Method used to scale the given size according to the camera's ratio, i.e.
    * zooming state.
    *
    * @param  {number} size - The size to scale (node size, edge thickness etc.).
    * @return {number}     - The scaled size.
    */
   Sigma.prototype.scaleSize = function (size) {
      return size / this.cameraSizeRatio;
   };
   /**
    * Method that returns the collection of all used canvases.
    * At the moment, the instantiated canvases are the following, and in the
    * following order in the DOM:
    * - `edges`
    * - `nodes`
    * - `edgeLabels`
    * - `labels`
    * - `hovers`
    * - `hoverNodes`
    * - `mouse`
    *
    * @return {PlainObject<HTMLCanvasElement>} - The collection of canvases.
    */
   Sigma.prototype.getCanvases = function () {
      return __assign({}, this.elements);
   };
   return Sigma;
}(types_1.TypedEventEmitter));
exports["default"] = Sigma;


/***/ }),
/* 2 */
/***/ ((module) => {

/**
 * Extend function
 * ================
 *
 * Function used to push a bunch of values into an array at once.
 *
 * Its strategy is to mutate target array's length then setting the new indices
 * to be the values to add.
 *
 * A benchmark proved that it is faster than the following strategies:
 *   1) `array.push.apply(array, values)`.
 *   2) A loop of pushes.
 *   3) `array = array.concat(values)`, obviously.
 *
 * Intuitively, this is correct because when adding a lot of elements, the
 * chosen strategies does not need to handle the `arguments` object to
 * execute #.apply's variadicity and because the array know its final length
 * at the beginning, avoiding potential multiple reallocations of the underlying
 * contiguous array. Some engines may be able to optimize the loop of push
 * operations but empirically they don't seem to do so.
 */

/**
 * Extends the target array with the given values.
 *
 * @param  {array} array  - Target array.
 * @param  {array} values - Values to add.
 */
module.exports = function extend(array, values) {
   var l2 = values.length;

   if (l2 === 0)
      return;

   var l1 = array.length;

   array.length += l2;

   for (var i = 0; i < l2; i++)
      array[l1 + i] = values[i];
};


/***/ }),
/* 3 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
/**
 * Sigma.js Camera Class
 * ======================
 *
 * Class designed to store camera information & used to update it.
 * @module
 */
var animate_1 = __webpack_require__(4);
var easings_1 = __importDefault(__webpack_require__(9));
var utils_1 = __webpack_require__(5);
var types_1 = __webpack_require__(10);
/**
 * Defaults.
 */
var DEFAULT_ZOOMING_RATIO = 1.5;
/**
 * Camera class
 *
 * @constructor
 */
var Camera = /** @class */ (function (_super) {
   __extends(Camera, _super);
   function Camera() {
      var _this = _super.call(this) || this;
      _this.x = 0.5;
      _this.y = 0.5;
      _this.angle = 0;
      _this.ratio = 1;
      _this.minRatio = null;
      _this.maxRatio = null;
      _this.nextFrame = null;
      _this.previousState = null;
      _this.enabled = true;
      // State
      _this.previousState = _this.getState();
      return _this;
   }
   /**
    * Static method used to create a Camera object with a given state.
    *
    * @param state
    * @return {Camera}
    */
   Camera.from = function (state) {
      var camera = new Camera();
      return camera.setState(state);
   };
   /**
    * Method used to enable the camera.
    *
    * @return {Camera}
    */
   Camera.prototype.enable = function () {
      this.enabled = true;
      return this;
   };
   /**
    * Method used to disable the camera.
    *
    * @return {Camera}
    */
   Camera.prototype.disable = function () {
      this.enabled = false;
      return this;
   };
   /**
    * Method used to retrieve the camera's current state.
    *
    * @return {object}
    */
   Camera.prototype.getState = function () {
      return {
         x: this.x,
         y: this.y,
         angle: this.angle,
         ratio: this.ratio,
      };
   };
   /**
    * Method used to check whether the camera has the given state.
    *
    * @return {object}
    */
   Camera.prototype.hasState = function (state) {
      return this.x === state.x && this.y === state.y && this.ratio === state.ratio && this.angle === state.angle;
   };
   /**
    * Method used to retrieve the camera's previous state.
    *
    * @return {object}
    */
   Camera.prototype.getPreviousState = function () {
      var state = this.previousState;
      if (!state)
         return null;
      return {
         x: state.x,
         y: state.y,
         angle: state.angle,
         ratio: state.ratio,
      };
   };
   /**
    * Method used to check minRatio and maxRatio values.
    *
    * @param ratio
    * @return {number}
    */
   Camera.prototype.getBoundedRatio = function (ratio) {
      var r = ratio;
      if (typeof this.minRatio === "number")
         r = Math.max(r, this.minRatio);
      if (typeof this.maxRatio === "number")
         r = Math.min(r, this.maxRatio);
      return r;
   };
   /**
    * Method used to check various things to return a legit state candidate.
    *
    * @param state
    * @return {object}
    */
   Camera.prototype.validateState = function (state) {
      var validatedState = {};
      if (typeof state.x === "number")
         validatedState.x = state.x;
      if (typeof state.y === "number")
         validatedState.y = state.y;
      if (typeof state.angle === "number")
         validatedState.angle = state.angle;
      if (typeof state.ratio === "number")
         validatedState.ratio = this.getBoundedRatio(state.ratio);
      return validatedState;
   };
   /**
    * Method used to check whether the camera is currently being animated.
    *
    * @return {boolean}
    */
   Camera.prototype.isAnimated = function () {
      return !!this.nextFrame;
   };
   /**
    * Method used to set the camera's state.
    *
    * @param  {object} state - New state.
    * @return {Camera}
    */
   Camera.prototype.setState = function (state) {
      if (!this.enabled)
         return this;
      // TODO: update by function
      // Keeping track of last state
      this.previousState = this.getState();
      var validState = this.validateState(state);
      if (typeof validState.x === "number")
         this.x = validState.x;
      if (typeof validState.y === "number")
         this.y = validState.y;
      if (typeof validState.angle === "number")
         this.angle = validState.angle;
      if (typeof validState.ratio === "number")
         this.ratio = validState.ratio;
      // Emitting
      if (!this.hasState(this.previousState))
         this.emit("updated", this.getState());
      return this;
   };
   /**
    * Method used to update the camera's state using a function.
    *
    * @param  {function} updater - Updated function taking current state and
    *                       returning next state.
    * @return {Camera}
    */
   Camera.prototype.updateState = function (updater) {
      this.setState(updater(this.getState()));
      return this;
   };
   /**
    * Method used to animate the camera.
    *
    * @param  {object}               state     - State to reach eventually.
    * @param  {object}               opts      - Options:
    * @param  {number}                 duration - Duration of the animation.
    * @param  {string | number => number}   easing   - Easing function or name of an existing one
    * @param  {function}              callback   - Callback
    */
   Camera.prototype.animate = function (state, opts, callback) {
      var _this = this;
      if (!this.enabled)
         return;
      var options = Object.assign({}, animate_1.ANIMATE_DEFAULTS, opts);
      var validState = this.validateState(state);
      var easing = typeof options.easing === "function" ? options.easing : easings_1.default[options.easing];
      // State
      var start = Date.now(), initialState = this.getState();
      // Function performing the animation
      var fn = function () {
         var t = (Date.now() - start) / options.duration;
         // The animation is over:
         if (t >= 1) {
            _this.nextFrame = null;
            _this.setState(validState);
            if (_this.animationCallback) {
               _this.animationCallback.call(null);
               _this.animationCallback = undefined;
            }
            return;
         }
         var coefficient = easing(t);
         var newState = {};
         if (typeof validState.x === "number")
            newState.x = initialState.x + (validState.x - initialState.x) * coefficient;
         if (typeof validState.y === "number")
            newState.y = initialState.y + (validState.y - initialState.y) * coefficient;
         if (typeof validState.angle === "number")
            newState.angle = initialState.angle + (validState.angle - initialState.angle) * coefficient;
         if (typeof validState.ratio === "number")
            newState.ratio = initialState.ratio + (validState.ratio - initialState.ratio) * coefficient;
         _this.setState(newState);
         _this.nextFrame = (0, utils_1.requestFrame)(fn);
      };
      if (this.nextFrame) {
         (0, utils_1.cancelFrame)(this.nextFrame);
         if (this.animationCallback)
            this.animationCallback.call(null);
         this.nextFrame = (0, utils_1.requestFrame)(fn);
      }
      else {
         fn();
      }
      this.animationCallback = callback;
   };
   /**
    * Method used to zoom the camera.
    *
    * @param  {number|object} factorOrOptions - Factor or options.
    * @return {function}
    */
   Camera.prototype.animatedZoom = function (factorOrOptions) {
      if (!factorOrOptions) {
         this.animate({ ratio: this.ratio / DEFAULT_ZOOMING_RATIO });
      }
      else {
         if (typeof factorOrOptions === "number")
            return this.animate({ ratio: this.ratio / factorOrOptions });
         else
            this.animate({
               ratio: this.ratio / (factorOrOptions.factor || DEFAULT_ZOOMING_RATIO),
            }, factorOrOptions);
      }
   };
   /**
    * Method used to unzoom the camera.
    *
    * @param  {number|object} factorOrOptions - Factor or options.
    */
   Camera.prototype.animatedUnzoom = function (factorOrOptions) {
      if (!factorOrOptions) {
         this.animate({ ratio: this.ratio * DEFAULT_ZOOMING_RATIO });
      }
      else {
         if (typeof factorOrOptions === "number")
            return this.animate({ ratio: this.ratio * factorOrOptions });
         else
            this.animate({
               ratio: this.ratio * (factorOrOptions.factor || DEFAULT_ZOOMING_RATIO),
            }, factorOrOptions);
      }
   };
   /**
    * Method used to reset the camera.
    *
    * @param  {object} options - Options.
    */
   Camera.prototype.animatedReset = function (options) {
      this.animate({
         x: 0.5,
         y: 0.5,
         ratio: 1,
         angle: 0,
      }, options);
   };
   /**
    * Returns a new Camera instance, with the same state as the current camera.
    *
    * @return {Camera}
    */
   Camera.prototype.copy = function () {
      return Camera.from(this.getState());
   };
   return Camera;
}(types_1.TypedEventEmitter));
exports["default"] = Camera;


/***/ }),
/* 4 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.animateNodes = exports.ANIMATE_DEFAULTS = void 0;
var index_1 = __webpack_require__(5);
var easings_1 = __importDefault(__webpack_require__(9));
exports.ANIMATE_DEFAULTS = {
   easing: "quadraticInOut",
   duration: 150,
};
/**
 * Function used to animate the nodes.
 */
function animateNodes(graph, targets, opts, callback) {
   var options = Object.assign({}, exports.ANIMATE_DEFAULTS, opts);
   var easing = typeof options.easing === "function" ? options.easing : easings_1.default[options.easing];
   var start = Date.now();
   var startPositions = {};
   for (var node in targets) {
      var attrs = targets[node];
      startPositions[node] = {};
      for (var k in attrs)
         startPositions[node][k] = graph.getNodeAttribute(node, k);
   }
   var frame = null;
   var step = function () {
      frame = null;
      var p = (Date.now() - start) / options.duration;
      if (p >= 1) {
         // Animation is done
         for (var node in targets) {
            var attrs = targets[node];
            // We use given values to avoid precision issues and for convenience
            for (var k in attrs)
               graph.setNodeAttribute(node, k, attrs[k]);
         }
         if (typeof callback === "function")
            callback();
         return;
      }
      p = easing(p);
      for (var node in targets) {
         var attrs = targets[node];
         var s = startPositions[node];
         for (var k in attrs)
            graph.setNodeAttribute(node, k, attrs[k] * p + s[k] * (1 - p));
      }
      frame = (0, index_1.requestFrame)(step);
   };
   step();
   return function () {
      if (frame)
         (0, index_1.cancelFrame)(frame);
   };
}
exports.animateNodes = animateNodes;


/***/ }),
/* 5 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __read = (this && this.__read) || function (o, n) {
   var m = typeof Symbol === "function" && o[Symbol.iterator];
   if (!m) return o;
   var i = m.call(o), r, ar = [], e;
   try {
      while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
   }
   catch (error) { e = { error: error }; }
   finally {
      try {
         if (r && !r.done && (m = i["return"])) m.call(i);
      }
      finally { if (e) throw e.error; }
   }
   return ar;
};
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.validateGraph = exports.canUse32BitsIndices = exports.extractPixel = exports.getMatrixImpact = exports.matrixFromCamera = exports.getCorrectionRatio = exports.floatColor = exports.floatArrayColor = exports.parseColor = exports.zIndexOrdering = exports.createNormalizationFunction = exports.graphExtent = exports.getPixelRatio = exports.createElement = exports.cancelFrame = exports.requestFrame = exports.assignDeep = exports.assign = exports.isPlainObject = void 0;
var is_graph_1 = __importDefault(__webpack_require__(6));
var matrices_1 = __webpack_require__(7);
var data_1 = __webpack_require__(8);
/**
 * Checks whether the given value is a plain object.
 *
 * @param  {mixed}   value - Target value.
 * @return {boolean}
 */
// eslint-disable-next-line @typescript-eslint/no-explicit-any, @typescript-eslint/explicit-module-boundary-types
function isPlainObject(value) {
   return typeof value === "object" && value !== null && value.constructor === Object;
}
exports.isPlainObject = isPlainObject;
/**
 * Helper to use Object.assign with more than two objects.
 *
 * @param  {object} target      - First object.
 * @param  {object} [...objects] - Objects to merge.
 * @return {object}
 */
function assign(target) {
   var objects = [];
   for (var _i = 1; _i < arguments.length; _i++) {
      objects[_i - 1] = arguments[_i];
   }
   target = target || {};
   for (var i = 0, l = objects.length; i < l; i++) {
      var o = objects[i];
      if (!o)
         continue;
      Object.assign(target, o);
   }
   return target;
}
exports.assign = assign;
/**
 * Very simple recursive Object.assign-like function.
 *
 * @param  {object} target      - First object.
 * @param  {object} [...objects] - Objects to merge.
 * @return {object}
 */
function assignDeep(target) {
   var objects = [];
   for (var _i = 1; _i < arguments.length; _i++) {
      objects[_i - 1] = arguments[_i];
   }
   target = target || {};
   for (var i = 0, l = objects.length; i < l; i++) {
      var o = objects[i];
      if (!o)
         continue;
      for (var k in o) {
         if (isPlainObject(o[k])) {
            target[k] = assignDeep(target[k], o[k]);
         }
         else {
            target[k] = o[k];
         }
      }
   }
   return target;
}
exports.assignDeep = assignDeep;
/**
 * Just some dirty trick to make requestAnimationFrame and cancelAnimationFrame "work" in Node.js, for unit tests:
 */
exports.requestFrame = typeof requestAnimationFrame !== "undefined"
   ? function (callback) { return requestAnimationFrame(callback); }
   : function (callback) { return setTimeout(callback, 0); };
exports.cancelFrame = typeof cancelAnimationFrame !== "undefined"
   ? function (requestID) { return cancelAnimationFrame(requestID); }
   : function (requestID) { return clearTimeout(requestID); };
/**
 * Function used to create DOM elements easily.
 *
 * @param  {string} tag      - Tag name of the element to create.
 * @param  {object} style     - Styles map.
 * @param  {object} attributes - Attributes map.
 * @return {HTMLElement}
 */
function createElement(tag, style, attributes) {
   var element = document.createElement(tag);
   if (style) {
      for (var k in style) {
         element.style[k] = style[k];
      }
   }
   if (attributes) {
      for (var k in attributes) {
         element.setAttribute(k, attributes[k]);
      }
   }
   return element;
}
exports.createElement = createElement;
/**
 * Function returning the browser's pixel ratio.
 *
 * @return {number}
 */
function getPixelRatio() {
   if (typeof window.devicePixelRatio !== "undefined")
      return window.devicePixelRatio;
   return 1;
}
exports.getPixelRatio = getPixelRatio;
/**
 * Function returning the graph's node extent in x & y.
 *
 * @param  {Graph}
 * @return {object}
 */
function graphExtent(graph) {
   if (!graph.order)
      return { x: [0, 1], y: [0, 1] };
   var xMin = Infinity;
   var xMax = -Infinity;
   var yMin = Infinity;
   var yMax = -Infinity;
   graph.forEachNode(function (_, attr) {
      var x = attr.x, y = attr.y;
      if (x < xMin)
         xMin = x;
      if (x > xMax)
         xMax = x;
      if (y < yMin)
         yMin = y;
      if (y > yMax)
         yMax = y;
   });
   return { x: [xMin, xMax], y: [yMin, yMax] };
}
exports.graphExtent = graphExtent;
function createNormalizationFunction(extent) {
   var _a = __read(extent.x, 2), minX = _a[0], maxX = _a[1], _b = __read(extent.y, 2), minY = _b[0], maxY = _b[1];
   var ratio = Math.max(maxX - minX, maxY - minY), dX = (maxX + minX) / 2, dY = (maxY + minY) / 2;
   if (ratio === 0 || Math.abs(ratio) === Infinity || isNaN(ratio))
      ratio = 1;
   if (isNaN(dX))
      dX = 0;
   if (isNaN(dY))
      dY = 0;
   var fn = function (data) {
      return {
         x: 0.5 + (data.x - dX) / ratio,
         y: 0.5 + (data.y - dY) / ratio,
      };
   };
   // TODO: possibility to apply this in batch over array of indices
   fn.applyTo = function (data) {
      data.x = 0.5 + (data.x - dX) / ratio;
      data.y = 0.5 + (data.y - dY) / ratio;
   };
   fn.inverse = function (data) {
      return {
         x: dX + ratio * (data.x - 0.5),
         y: dY + ratio * (data.y - 0.5),
      };
   };
   fn.ratio = ratio;
   return fn;
}
exports.createNormalizationFunction = createNormalizationFunction;
/**
 * Function ordering the given elements in reverse z-order so they drawn
 * the correct way.
 *
 * @param  {number}   extent   - [min, max] z values.
 * @param  {function} getter   - Z attribute getter function.
 * @param  {array}   elements - The array to sort.
 * @return {array} - The sorted array.
 */
function zIndexOrdering(extent, getter, elements) {
   // If k is > n, we'll use a standard sort
   return elements.sort(function (a, b) {
      var zA = getter(a) || 0, zB = getter(b) || 0;
      if (zA < zB)
         return -1;
      if (zA > zB)
         return 1;
      return 0;
   });
   // TODO: counting sort optimization
}
exports.zIndexOrdering = zIndexOrdering;
/**
 * WebGL utils
 * ===========
 */
/**
 * Memoized function returning a float-encoded color from various string
 * formats describing colors.
 */
var INT8 = new Int8Array(4);
var INT32 = new Int32Array(INT8.buffer, 0, 1);
var FLOAT32 = new Float32Array(INT8.buffer, 0, 1);
var RGBA_TEST_REGEX = /^\s*rgba?\s*\(/;
var RGBA_EXTRACT_REGEX = /^\s*rgba?\s*\(\s*([0-9]*)\s*,\s*([0-9]*)\s*,\s*([0-9]*)(?:\s*,\s*(.*)?)?\)\s*$/;
function parseColor(val) {
   var r = 0; // byte
   var g = 0; // byte
   var b = 0; // byte
   var a = 1; // float
   // Handling hexadecimal notation
   if (val[0] === "#") {
      if (val.length === 4) {
         r = parseInt(val.charAt(1) + val.charAt(1), 16);
         g = parseInt(val.charAt(2) + val.charAt(2), 16);
         b = parseInt(val.charAt(3) + val.charAt(3), 16);
      }
      else {
         r = parseInt(val.charAt(1) + val.charAt(2), 16);
         g = parseInt(val.charAt(3) + val.charAt(4), 16);
         b = parseInt(val.charAt(5) + val.charAt(6), 16);
      }
      if (val.length === 9) {
         a = parseInt(val.charAt(7) + val.charAt(8), 16) / 255;
      }
   }
   // Handling rgb notation
   else if (RGBA_TEST_REGEX.test(val)) {
      var match = val.match(RGBA_EXTRACT_REGEX);
      if (match) {
         r = +match[1];
         g = +match[2];
         b = +match[3];
         if (match[4])
            a = +match[4];
      }
   }
   return { r: r, g: g, b: b, a: a };
}
exports.parseColor = parseColor;
var FLOAT_COLOR_CACHE = {};
for (var htmlColor in data_1.HTML_COLORS) {
   FLOAT_COLOR_CACHE[htmlColor] = floatColor(data_1.HTML_COLORS[htmlColor]);
   // Replicating cache for hex values for free
   FLOAT_COLOR_CACHE[data_1.HTML_COLORS[htmlColor]] = FLOAT_COLOR_CACHE[htmlColor];
}
function floatArrayColor(val) {
   val = data_1.HTML_COLORS[val] || val;
   // NOTE: this variant is not cached because it is mostly used for uniforms
   var _a = parseColor(val), r = _a.r, g = _a.g, b = _a.b, a = _a.a;
   return new Float32Array([r / 255, g / 255, b / 255, a]);
}
exports.floatArrayColor = floatArrayColor;
function floatColor(val) {
   // If the color is already computed, we yield it
   if (typeof FLOAT_COLOR_CACHE[val] !== "undefined")
      return FLOAT_COLOR_CACHE[val];
   var parsed = parseColor(val);
   var r = parsed.r, g = parsed.g, b = parsed.b;
   var a = parsed.a;
   a = (a * 255) | 0;
   INT32[0] = ((a << 24) | (b << 16) | (g << 8) | r) & 0xfeffffff;
   var color = FLOAT32[0];
   FLOAT_COLOR_CACHE[val] = color;
   return color;
}
exports.floatColor = floatColor;
/**
 * In sigma, the graph is normalized into a [0, 1], [0, 1] square, before being given to the various renderers. This
 * helps dealing with quadtree in particular.
 * But at some point, we need to rescale it so that it takes the best place in the screen, ie. we always want to see two
 * nodes "touching" opposite sides of the graph, with the camera being at its default state.
 *
 * This function determines this ratio.
 */
function getCorrectionRatio(viewportDimensions, graphDimensions) {
   var viewportRatio = viewportDimensions.height / viewportDimensions.width;
   var graphRatio = graphDimensions.height / graphDimensions.width;
   // If the stage and the graphs are in different directions (such as the graph being wider that tall while the stage
   // is taller than wide), we can stop here to have indeed nodes touching opposite sides:
   if ((viewportRatio < 1 && graphRatio > 1) || (viewportRatio > 1 && graphRatio < 1)) {
      return 1;
   }
   // Else, we need to fit the graph inside the stage:
   // 1. If the graph is "squarer" (ie. with a ratio closer to 1), we need to make the largest sides touch;
   // 2. If the stage is "squarer", we need to make the smallest sides touch.
   return Math.min(Math.max(graphRatio, 1 / graphRatio), Math.max(1 / viewportRatio, viewportRatio));
}
exports.getCorrectionRatio = getCorrectionRatio;
/**
 * Function returning a matrix from the current state of the camera.
 */
// TODO: it's possible to optimize this drastically!
function matrixFromCamera(state, viewportDimensions, graphDimensions, padding, inverse) {
   var angle = state.angle, ratio = state.ratio, x = state.x, y = state.y;
   var width = viewportDimensions.width, height = viewportDimensions.height;
   var matrix = (0, matrices_1.identity)();
   var smallestDimension = Math.min(width, height) - 2 * padding;
   var correctionRatio = getCorrectionRatio(viewportDimensions, graphDimensions);
   if (!inverse) {
      (0, matrices_1.multiply)(matrix, (0, matrices_1.scale)((0, matrices_1.identity)(), 2 * (smallestDimension / width) * correctionRatio, 2 * (smallestDimension / height) * correctionRatio));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.rotate)((0, matrices_1.identity)(), -angle));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.scale)((0, matrices_1.identity)(), 1 / ratio));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.translate)((0, matrices_1.identity)(), -x, -y));
   }
   else {
      (0, matrices_1.multiply)(matrix, (0, matrices_1.translate)((0, matrices_1.identity)(), x, y));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.scale)((0, matrices_1.identity)(), ratio));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.rotate)((0, matrices_1.identity)(), angle));
      (0, matrices_1.multiply)(matrix, (0, matrices_1.scale)((0, matrices_1.identity)(), width / smallestDimension / 2 / correctionRatio, height / smallestDimension / 2 / correctionRatio));
   }
   return matrix;
}
exports.matrixFromCamera = matrixFromCamera;
/**
 * All these transformations we apply on the matrix to get it rescale the graph
 * as we want make it very hard to get pixel-perfect distances in WebGL. This
 * function returns a factor that properly cancels the matrix effect on lengths.
 *
 * [jacomyal]
 * To be fully honest, I can't really explain happens here... I notice that the
 * following ratio works (ie. it correctly compensates the matrix impact on all
 * camera states I could try):
 * > `R = size(V) / size(M * V) / W`
 * as long as `M * V` is in the direction of W (ie. parallel to (Ox)). It works
 * as well with H and a vector that transforms into something parallel to (Oy).
 *
 * Also, note that we use `angle` and not `-angle` (that would seem logical,
 * since we want to anticipate the rotation), because of the fact that in WebGL,
 * the image is vertically swapped.
 */
function getMatrixImpact(matrix, cameraState, viewportDimensions) {
   var _a = (0, matrices_1.multiplyVec2)(matrix, { x: Math.cos(cameraState.angle), y: Math.sin(cameraState.angle) }, 0), x = _a.x, y = _a.y;
   return 1 / Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2)) / viewportDimensions.width;
}
exports.getMatrixImpact = getMatrixImpact;
/**
 * Function extracting the color at the given pixel.
 */
function extractPixel(gl, x, y, array) {
   var data = array || new Uint8Array(4);
   gl.readPixels(x, y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, data);
   return data;
}
exports.extractPixel = extractPixel;
/**
 * Function used to know whether given webgl context can use 32 bits indices.
 */
function canUse32BitsIndices(gl) {
   var webgl2 = typeof WebGL2RenderingContext !== "undefined" && gl instanceof WebGL2RenderingContext;
   return webgl2 || !!gl.getExtension("OES_element_index_uint");
}
exports.canUse32BitsIndices = canUse32BitsIndices;
/**
 * Check if the graph variable is a valid graph, and if sigma can render it.
 */
function validateGraph(graph) {
   // check if it's a valid graphology instance
   if (!(0, is_graph_1.default)(graph))
      throw new Error("Sigma: invalid graph instance.");
   // check if nodes have x/y attributes
   graph.forEachNode(function (key, attributes) {
      if (!Number.isFinite(attributes.x) || !Number.isFinite(attributes.y)) {
         throw new Error("Sigma: Coordinates of node ".concat(key, " are invalid. A node must have a numeric 'x' and 'y' attribute."));
      }
   });
}
exports.validateGraph = validateGraph;


/***/ }),
/* 6 */
/***/ ((module) => {

/**
 * Graphology isGraph
 * ===================
 *
 * Very simple function aiming at ensuring the given variable is a
 * graphology instance.
 */

/**
 * Checking the value is a graphology instance.
 *
 * @param  {any}    value - Target value.
 * @return {boolean}
 */
module.exports = function isGraph(value) {
   return (
      value !== null &&
      typeof value === 'object' &&
      typeof value.addUndirectedEdgeWithKey === 'function' &&
      typeof value.dropNode === 'function' &&
      typeof value.multi === 'boolean'
   );
};


/***/ }),
/* 7 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.multiplyVec2 = exports.multiply = exports.translate = exports.rotate = exports.scale = exports.identity = void 0;
function identity() {
   return Float32Array.of(1, 0, 0, 0, 1, 0, 0, 0, 1);
}
exports.identity = identity;
// TODO: optimize
function scale(m, x, y) {
   m[0] = x;
   m[4] = typeof y === "number" ? y : x;
   return m;
}
exports.scale = scale;
function rotate(m, r) {
   var s = Math.sin(r), c = Math.cos(r);
   m[0] = c;
   m[1] = s;
   m[3] = -s;
   m[4] = c;
   return m;
}
exports.rotate = rotate;
function translate(m, x, y) {
   m[6] = x;
   m[7] = y;
   return m;
}
exports.translate = translate;
function multiply(a, b) {
   var a00 = a[0], a01 = a[1], a02 = a[2];
   var a10 = a[3], a11 = a[4], a12 = a[5];
   var a20 = a[6], a21 = a[7], a22 = a[8];
   var b00 = b[0], b01 = b[1], b02 = b[2];
   var b10 = b[3], b11 = b[4], b12 = b[5];
   var b20 = b[6], b21 = b[7], b22 = b[8];
   a[0] = b00 * a00 + b01 * a10 + b02 * a20;
   a[1] = b00 * a01 + b01 * a11 + b02 * a21;
   a[2] = b00 * a02 + b01 * a12 + b02 * a22;
   a[3] = b10 * a00 + b11 * a10 + b12 * a20;
   a[4] = b10 * a01 + b11 * a11 + b12 * a21;
   a[5] = b10 * a02 + b11 * a12 + b12 * a22;
   a[6] = b20 * a00 + b21 * a10 + b22 * a20;
   a[7] = b20 * a01 + b21 * a11 + b22 * a21;
   a[8] = b20 * a02 + b21 * a12 + b22 * a22;
   return a;
}
exports.multiply = multiply;
function multiplyVec2(a, b, z) {
   if (z === void 0) { z = 1; }
   var a00 = a[0];
   var a01 = a[1];
   var a10 = a[3];
   var a11 = a[4];
   var a20 = a[6];
   var a21 = a[7];
   var b0 = b.x;
   var b1 = b.y;
   return { x: b0 * a00 + b1 * a10 + a20 * z, y: b0 * a01 + b1 * a11 + a21 * z };
}
exports.multiplyVec2 = multiplyVec2;


/***/ }),
/* 8 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.HTML_COLORS = void 0;
exports.HTML_COLORS = {
   black: "#000000",
   silver: "#C0C0C0",
   gray: "#808080",
   grey: "#808080",
   white: "#FFFFFF",
   maroon: "#800000",
   red: "#FF0000",
   purple: "#800080",
   fuchsia: "#FF00FF",
   green: "#008000",
   lime: "#00FF00",
   olive: "#808000",
   yellow: "#FFFF00",
   navy: "#000080",
   blue: "#0000FF",
   teal: "#008080",
   aqua: "#00FFFF",
   darkblue: "#00008B",
   mediumblue: "#0000CD",
   darkgreen: "#006400",
   darkcyan: "#008B8B",
   deepskyblue: "#00BFFF",
   darkturquoise: "#00CED1",
   mediumspringgreen: "#00FA9A",
   springgreen: "#00FF7F",
   cyan: "#00FFFF",
   midnightblue: "#191970",
   dodgerblue: "#1E90FF",
   lightseagreen: "#20B2AA",
   forestgreen: "#228B22",
   seagreen: "#2E8B57",
   darkslategray: "#2F4F4F",
   darkslategrey: "#2F4F4F",
   limegreen: "#32CD32",
   mediumseagreen: "#3CB371",
   turquoise: "#40E0D0",
   royalblue: "#4169E1",
   steelblue: "#4682B4",
   darkslateblue: "#483D8B",
   mediumturquoise: "#48D1CC",
   indigo: "#4B0082",
   darkolivegreen: "#556B2F",
   cadetblue: "#5F9EA0",
   cornflowerblue: "#6495ED",
   rebeccapurple: "#663399",
   mediumaquamarine: "#66CDAA",
   dimgray: "#696969",
   dimgrey: "#696969",
   slateblue: "#6A5ACD",
   olivedrab: "#6B8E23",
   slategray: "#708090",
   slategrey: "#708090",
   lightslategray: "#778899",
   lightslategrey: "#778899",
   mediumslateblue: "#7B68EE",
   lawngreen: "#7CFC00",
   chartreuse: "#7FFF00",
   aquamarine: "#7FFFD4",
   skyblue: "#87CEEB",
   lightskyblue: "#87CEFA",
   blueviolet: "#8A2BE2",
   darkred: "#8B0000",
   darkmagenta: "#8B008B",
   saddlebrown: "#8B4513",
   darkseagreen: "#8FBC8F",
   lightgreen: "#90EE90",
   mediumpurple: "#9370DB",
   darkviolet: "#9400D3",
   palegreen: "#98FB98",
   darkorchid: "#9932CC",
   yellowgreen: "#9ACD32",
   sienna: "#A0522D",
   brown: "#A52A2A",
   darkgray: "#A9A9A9",
   darkgrey: "#A9A9A9",
   lightblue: "#ADD8E6",
   greenyellow: "#ADFF2F",
   paleturquoise: "#AFEEEE",
   lightsteelblue: "#B0C4DE",
   powderblue: "#B0E0E6",
   firebrick: "#B22222",
   darkgoldenrod: "#B8860B",
   mediumorchid: "#BA55D3",
   rosybrown: "#BC8F8F",
   darkkhaki: "#BDB76B",
   mediumvioletred: "#C71585",
   indianred: "#CD5C5C",
   peru: "#CD853F",
   chocolate: "#D2691E",
   tan: "#D2B48C",
   lightgray: "#D3D3D3",
   lightgrey: "#D3D3D3",
   thistle: "#D8BFD8",
   orchid: "#DA70D6",
   goldenrod: "#DAA520",
   palevioletred: "#DB7093",
   crimson: "#DC143C",
   gainsboro: "#DCDCDC",
   plum: "#DDA0DD",
   burlywood: "#DEB887",
   lightcyan: "#E0FFFF",
   lavender: "#E6E6FA",
   darksalmon: "#E9967A",
   violet: "#EE82EE",
   palegoldenrod: "#EEE8AA",
   lightcoral: "#F08080",
   khaki: "#F0E68C",
   aliceblue: "#F0F8FF",
   honeydew: "#F0FFF0",
   azure: "#F0FFFF",
   sandybrown: "#F4A460",
   wheat: "#F5DEB3",
   beige: "#F5F5DC",
   whitesmoke: "#F5F5F5",
   mintcream: "#F5FFFA",
   ghostwhite: "#F8F8FF",
   salmon: "#FA8072",
   antiquewhite: "#FAEBD7",
   linen: "#FAF0E6",
   lightgoldenrodyellow: "#FAFAD2",
   oldlace: "#FDF5E6",
   magenta: "#FF00FF",
   deeppink: "#FF1493",
   orangered: "#FF4500",
   tomato: "#FF6347",
   hotpink: "#FF69B4",
   coral: "#FF7F50",
   darkorange: "#FF8C00",
   lightsalmon: "#FFA07A",
   orange: "#FFA500",
   lightpink: "#FFB6C1",
   pink: "#FFC0CB",
   gold: "#FFD700",
   peachpuff: "#FFDAB9",
   navajowhite: "#FFDEAD",
   moccasin: "#FFE4B5",
   bisque: "#FFE4C4",
   mistyrose: "#FFE4E1",
   blanchedalmond: "#FFEBCD",
   papayawhip: "#FFEFD5",
   lavenderblush: "#FFF0F5",
   seashell: "#FFF5EE",
   cornsilk: "#FFF8DC",
   lemonchiffon: "#FFFACD",
   floralwhite: "#FFFAF0",
   snow: "#FFFAFA",
   lightyellow: "#FFFFE0",
   ivory: "#FFFFF0",
};


/***/ }),
/* 9 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.cubicInOut = exports.cubicOut = exports.cubicIn = exports.quadraticInOut = exports.quadraticOut = exports.quadraticIn = exports.linear = void 0;
/**
 * Sigma.js Easings
 * =================
 *
 * Handy collection of easing functions.
 * @module
 */
var linear = function (k) { return k; };
exports.linear = linear;
var quadraticIn = function (k) { return k * k; };
exports.quadraticIn = quadraticIn;
var quadraticOut = function (k) { return k * (2 - k); };
exports.quadraticOut = quadraticOut;
var quadraticInOut = function (k) {
   if ((k *= 2) < 1)
      return 0.5 * k * k;
   return -0.5 * (--k * (k - 2) - 1);
};
exports.quadraticInOut = quadraticInOut;
var cubicIn = function (k) { return k * k * k; };
exports.cubicIn = cubicIn;
var cubicOut = function (k) { return --k * k * k + 1; };
exports.cubicOut = cubicOut;
var cubicInOut = function (k) {
   if ((k *= 2) < 1)
      return 0.5 * k * k * k;
   return 0.5 * ((k -= 2) * k * k + 2);
};
exports.cubicInOut = cubicInOut;
var easings = {
   linear: exports.linear,
   quadraticIn: exports.quadraticIn,
   quadraticOut: exports.quadraticOut,
   quadraticInOut: exports.quadraticInOut,
   cubicIn: exports.cubicIn,
   cubicOut: exports.cubicOut,
   cubicInOut: exports.cubicInOut,
};
exports["default"] = easings;


/***/ }),
/* 10 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.TypedEventEmitter = void 0;
/**
 * Sigma.js Types
 * ===============
 *
 * Various type declarations used throughout the library.
 * @module
 */
var events_1 = __webpack_require__(11);
var TypedEventEmitter = /** @class */ (function (_super) {
   __extends(TypedEventEmitter, _super);
   function TypedEventEmitter() {
      var _this = _super.call(this) || this;
      _this.rawEmitter = _this;
      return _this;
   }
   return TypedEventEmitter;
}(events_1.EventEmitter));
exports.TypedEventEmitter = TypedEventEmitter;


/***/ }),
/* 11 */
/***/ ((module) => {

"use strict";
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.



var R = typeof Reflect === 'object' ? Reflect : null
var ReflectApply = R && typeof R.apply === 'function'
   ? R.apply
   : function ReflectApply(target, receiver, args) {
      return Function.prototype.apply.call(target, receiver, args);
   }

var ReflectOwnKeys
if (R && typeof R.ownKeys === 'function') {
   ReflectOwnKeys = R.ownKeys
} else if (Object.getOwnPropertySymbols) {
   ReflectOwnKeys = function ReflectOwnKeys(target) {
      return Object.getOwnPropertyNames(target)
         .concat(Object.getOwnPropertySymbols(target));
   };
} else {
   ReflectOwnKeys = function ReflectOwnKeys(target) {
      return Object.getOwnPropertyNames(target);
   };
}

function ProcessEmitWarning(warning) {
   if (console && console.warn) console.warn(warning);
}

var NumberIsNaN = Number.isNaN || function NumberIsNaN(value) {
   return value !== value;
}

function EventEmitter() {
   EventEmitter.init.call(this);
}
module.exports = EventEmitter;
module.exports.once = once;

// Backwards-compat with node 0.10.x
EventEmitter.EventEmitter = EventEmitter;

EventEmitter.prototype._events = undefined;
EventEmitter.prototype._eventsCount = 0;
EventEmitter.prototype._maxListeners = undefined;

// By default EventEmitters will print a warning if more than 10 listeners are
// added to it. This is a useful default which helps finding memory leaks.
var defaultMaxListeners = 10;

function checkListener(listener) {
   if (typeof listener !== 'function') {
      throw new TypeError('The "listener" argument must be of type Function. Received type ' + typeof listener);
   }
}

Object.defineProperty(EventEmitter, 'defaultMaxListeners', {
   enumerable: true,
   get: function() {
      return defaultMaxListeners;
   },
   set: function(arg) {
      if (typeof arg !== 'number' || arg < 0 || NumberIsNaN(arg)) {
         throw new RangeError('The value of "defaultMaxListeners" is out of range. It must be a non-negative number. Received ' + arg + '.');
      }
      defaultMaxListeners = arg;
   }
});

EventEmitter.init = function() {
   if ( this._events === undefined ||
         this._events === Object.getPrototypeOf(this)._events) {
      this._events = Object.create(null);
      this._eventsCount = 0;
   }

   this._maxListeners = this._maxListeners || undefined;
};

// Obviously not all Emitters should be limited to 10. This function allows
// that to be increased. Set to zero for unlimited.
EventEmitter.prototype.setMaxListeners = function setMaxListeners(n) {
   if (typeof n !== 'number' || n < 0 || NumberIsNaN(n)) {
      throw new RangeError('The value of "n" is out of range. It must be a non-negative number. Received ' + n + '.');
   }
   this._maxListeners = n;
   return this;
};

function _getMaxListeners(that) {
   if (that._maxListeners === undefined)
      return EventEmitter.defaultMaxListeners;
   return that._maxListeners;
}

EventEmitter.prototype.getMaxListeners = function getMaxListeners() {
   return _getMaxListeners(this);
};

EventEmitter.prototype.emit = function emit(type) {
   var args = [];
   for (var i = 1; i < arguments.length; i++) args.push(arguments[i]);
   var doError = (type === 'error');

   var events = this._events;
   if (events !== undefined)
      doError = (doError && events.error === undefined);
   else if (!doError)
      return false;

   // If there is no 'error' event listener then throw.
   if (doError) {
      var er;
      if (args.length > 0)
         er = args[0];
      if (er instanceof Error) {
         // Note: The comments on the `throw` lines are intentional, they show
         // up in Node's output if this results in an unhandled exception.
         throw er; // Unhandled 'error' event
      }
      // At least give some kind of context to the user
      var err = new Error('Unhandled error.' + (er ? ' (' + er.message + ')' : ''));
      err.context = er;
      throw err; // Unhandled 'error' event
   }

   var handler = events[type];

   if (handler === undefined)
      return false;

   if (typeof handler === 'function') {
      ReflectApply(handler, this, args);
   } else {
      var len = handler.length;
      var listeners = arrayClone(handler, len);
      for (var i = 0; i < len; ++i)
         ReflectApply(listeners[i], this, args);
   }
   return true;
};

function _addListener(target, type, listener, prepend) {
   var m;
   var events;
   var existing;

   checkListener(listener);

   events = target._events;
   if (events === undefined) {
      events = target._events = Object.create(null);
      target._eventsCount = 0;
   } else {
      // To avoid recursion in the case that type === "newListener"! Before
      // adding it to the listeners, first emit "newListener".
      if (events.newListener !== undefined) {
         target.emit('newListener', type,
            listener.listener ? listener.listener : listener);

         // Re-assign `events` because a newListener handler could have caused the
         // this._events to be assigned to a new object
         events = target._events;
      }
      existing = events[type];
   }

   if (existing === undefined) {
      // Optimize the case of one listener. Don't need the extra array object.
      existing = events[type] = listener;
      ++target._eventsCount;
   } else {
      if (typeof existing === 'function') {
         // Adding the second element, need to change to array.
         existing = events[type] =
            prepend ? [listener, existing] : [existing, listener];
         // If we've already got an array, just append.
      } else if (prepend) {
         existing.unshift(listener);
      } else {
         existing.push(listener);
      }

      // Check for listener leak
      m = _getMaxListeners(target);
      if (m > 0 && existing.length > m && !existing.warned) {
         existing.warned = true;
         // No error code for this since it is a Warning
         // eslint-disable-next-line no-restricted-syntax
         var w = new Error('Possible EventEmitter memory leak detected. ' +
            existing.length + ' ' + String(type) + ' listeners ' +
            'added. Use emitter.setMaxListeners() to ' +
            'increase limit');
         w.name = 'MaxListenersExceededWarning';
         w.emitter = target;
         w.type = type;
         w.count = existing.length;
         ProcessEmitWarning(w);
      }
   }

   return target;
}

EventEmitter.prototype.addListener = function addListener(type, listener) {
   return _addListener(this, type, listener, false);
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;

EventEmitter.prototype.prependListener =
   function prependListener(type, listener) {
      return _addListener(this, type, listener, true);
   };

function onceWrapper() {
   if (!this.fired) {
      this.target.removeListener(this.type, this.wrapFn);
      this.fired = true;
      if (arguments.length === 0)
         return this.listener.call(this.target);
      return this.listener.apply(this.target, arguments);
   }
}

function _onceWrap(target, type, listener) {
   var state = { fired: false, wrapFn: undefined, target: target, type: type, listener: listener };
   var wrapped = onceWrapper.bind(state);
   wrapped.listener = listener;
   state.wrapFn = wrapped;
   return wrapped;
}

EventEmitter.prototype.once = function once(type, listener) {
   checkListener(listener);
   this.on(type, _onceWrap(this, type, listener));
   return this;
};

EventEmitter.prototype.prependOnceListener =
   function prependOnceListener(type, listener) {
      checkListener(listener);
      this.prependListener(type, _onceWrap(this, type, listener));
      return this;
   };

// Emits a 'removeListener' event if and only if the listener was removed.
EventEmitter.prototype.removeListener =
   function removeListener(type, listener) {
      var list, events, position, i, originalListener;

      checkListener(listener);

      events = this._events;
      if (events === undefined)
         return this;

      list = events[type];
      if (list === undefined)
         return this;

      if (list === listener || list.listener === listener) {
         if (--this._eventsCount === 0)
            this._events = Object.create(null);
         else {
            delete events[type];
            if (events.removeListener)
               this.emit('removeListener', type, list.listener || listener);
         }
      } else if (typeof list !== 'function') {
         position = -1;

         for (i = list.length - 1; i >= 0; i--) {
            if (list[i] === listener || list[i].listener === listener) {
               originalListener = list[i].listener;
               position = i;
               break;
            }
         }

         if (position < 0)
            return this;

         if (position === 0)
            list.shift();
         else {
            spliceOne(list, position);
         }

         if (list.length === 1)
            events[type] = list[0];

         if (events.removeListener !== undefined)
            this.emit('removeListener', type, originalListener || listener);
      }

      return this;
   };

EventEmitter.prototype.off = EventEmitter.prototype.removeListener;

EventEmitter.prototype.removeAllListeners =
   function removeAllListeners(type) {
      var listeners, events, i;

      events = this._events;
      if (events === undefined)
         return this;

      // not listening for removeListener, no need to emit
      if (events.removeListener === undefined) {
         if (arguments.length === 0) {
            this._events = Object.create(null);
            this._eventsCount = 0;
         } else if (events[type] !== undefined) {
            if (--this._eventsCount === 0)
               this._events = Object.create(null);
            else
               delete events[type];
         }
         return this;
      }

      // emit removeListener for all listeners on all events
      if (arguments.length === 0) {
         var keys = Object.keys(events);
         var key;
         for (i = 0; i < keys.length; ++i) {
            key = keys[i];
            if (key === 'removeListener') continue;
            this.removeAllListeners(key);
         }
         this.removeAllListeners('removeListener');
         this._events = Object.create(null);
         this._eventsCount = 0;
         return this;
      }

      listeners = events[type];

      if (typeof listeners === 'function') {
         this.removeListener(type, listeners);
      } else if (listeners !== undefined) {
         // LIFO order
         for (i = listeners.length - 1; i >= 0; i--) {
            this.removeListener(type, listeners[i]);
         }
      }

      return this;
   };

function _listeners(target, type, unwrap) {
   var events = target._events;

   if (events === undefined)
      return [];

   var evlistener = events[type];
   if (evlistener === undefined)
      return [];

   if (typeof evlistener === 'function')
      return unwrap ? [evlistener.listener || evlistener] : [evlistener];

   return unwrap ?
      unwrapListeners(evlistener) : arrayClone(evlistener, evlistener.length);
}

EventEmitter.prototype.listeners = function listeners(type) {
   return _listeners(this, type, true);
};

EventEmitter.prototype.rawListeners = function rawListeners(type) {
   return _listeners(this, type, false);
};

EventEmitter.listenerCount = function(emitter, type) {
   if (typeof emitter.listenerCount === 'function') {
      return emitter.listenerCount(type);
   } else {
      return listenerCount.call(emitter, type);
   }
};

EventEmitter.prototype.listenerCount = listenerCount;
function listenerCount(type) {
   var events = this._events;

   if (events !== undefined) {
      var evlistener = events[type];

      if (typeof evlistener === 'function') {
         return 1;
      } else if (evlistener !== undefined) {
         return evlistener.length;
      }
   }

   return 0;
}

EventEmitter.prototype.eventNames = function eventNames() {
   return this._eventsCount > 0 ? ReflectOwnKeys(this._events) : [];
};

function arrayClone(arr, n) {
   var copy = new Array(n);
   for (var i = 0; i < n; ++i)
      copy[i] = arr[i];
   return copy;
}

function spliceOne(list, index) {
   for (; index + 1 < list.length; index++)
      list[index] = list[index + 1];
   list.pop();
}

function unwrapListeners(arr) {
   var ret = new Array(arr.length);
   for (var i = 0; i < ret.length; ++i) {
      ret[i] = arr[i].listener || arr[i];
   }
   return ret;
}

function once(emitter, name) {
   return new Promise(function (resolve, reject) {
      function errorListener(err) {
         emitter.removeListener(name, resolver);
         reject(err);
      }

      function resolver() {
         if (typeof emitter.removeListener === 'function') {
            emitter.removeListener('error', errorListener);
         }
         resolve([].slice.call(arguments));
      };

      eventTargetAgnosticAddListener(emitter, name, resolver, { once: true });
      if (name !== 'error') {
         addErrorHandlerIfEventEmitter(emitter, errorListener, { once: true });
      }
   });
}

function addErrorHandlerIfEventEmitter(emitter, handler, flags) {
   if (typeof emitter.on === 'function') {
      eventTargetAgnosticAddListener(emitter, 'error', handler, flags);
   }
}

function eventTargetAgnosticAddListener(emitter, name, listener, flags) {
   if (typeof emitter.on === 'function') {
      if (flags.once) {
         emitter.once(name, listener);
      } else {
         emitter.on(name, listener);
      }
   } else if (typeof emitter.addEventListener === 'function') {
      // EventTarget does not have `error` event semantics like Node
      // EventEmitters, we do not listen for `error` events here.
      emitter.addEventListener(name, function wrapListener(arg) {
         // IE does not have builtin `{ once: true }` support so we
         // have to do it manually.
         if (flags.once) {
            emitter.removeEventListener(name, wrapListener);
         }
         listener(arg);
      });
   } else {
      throw new TypeError('The "emitter" argument must be of type EventEmitter. Received type ' + typeof emitter);
   }
}


/***/ }),
/* 12 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
   if (k2 === undefined) k2 = k;
   Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
   if (k2 === undefined) k2 = k;
   o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
   Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
   o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
   if (mod && mod.__esModule) return mod;
   var result = {};
   if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
   __setModuleDefault(result, mod);
   return result;
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var captor_1 = __importStar(__webpack_require__(13));
/**
 * Constants.
 */
var DRAG_TIMEOUT = 100;
var DRAGGED_EVENTS_TOLERANCE = 3;
var MOUSE_INERTIA_DURATION = 200;
var MOUSE_INERTIA_RATIO = 3;
var MOUSE_ZOOM_DURATION = 250;
var ZOOMING_RATIO = 1.7;
var DOUBLE_CLICK_TIMEOUT = 300;
var DOUBLE_CLICK_ZOOMING_RATIO = 2.2;
var DOUBLE_CLICK_ZOOMING_DURATION = 200;
/**
 * Mouse captor class.
 *
 * @constructor
 */
var MouseCaptor = /** @class */ (function (_super) {
   __extends(MouseCaptor, _super);
   function MouseCaptor(container, renderer) {
      var _this = _super.call(this, container, renderer) || this;
      // State
      _this.enabled = true;
      _this.draggedEvents = 0;
      _this.downStartTime = null;
      _this.lastMouseX = null;
      _this.lastMouseY = null;
      _this.isMouseDown = false;
      _this.isMoving = false;
      _this.movingTimeout = null;
      _this.startCameraState = null;
      _this.clicks = 0;
      _this.doubleClickTimeout = null;
      _this.currentWheelDirection = 0;
      // Binding methods
      _this.handleClick = _this.handleClick.bind(_this);
      _this.handleRightClick = _this.handleRightClick.bind(_this);
      _this.handleDown = _this.handleDown.bind(_this);
      _this.handleUp = _this.handleUp.bind(_this);
      _this.handleMove = _this.handleMove.bind(_this);
      _this.handleWheel = _this.handleWheel.bind(_this);
      _this.handleOut = _this.handleOut.bind(_this);
      // Binding events
      container.addEventListener("click", _this.handleClick, false);
      container.addEventListener("contextmenu", _this.handleRightClick, false);
      container.addEventListener("mousedown", _this.handleDown, false);
      container.addEventListener("wheel", _this.handleWheel, false);
      container.addEventListener("mouseout", _this.handleOut, false);
      document.addEventListener("mousemove", _this.handleMove, false);
      document.addEventListener("mouseup", _this.handleUp, false);
      return _this;
   }
   MouseCaptor.prototype.kill = function () {
      var container = this.container;
      container.removeEventListener("click", this.handleClick);
      container.removeEventListener("contextmenu", this.handleRightClick);
      container.removeEventListener("mousedown", this.handleDown);
      container.removeEventListener("wheel", this.handleWheel);
      container.removeEventListener("mouseout", this.handleOut);
      document.removeEventListener("mousemove", this.handleMove);
      document.removeEventListener("mouseup", this.handleUp);
   };
   MouseCaptor.prototype.handleClick = function (e) {
      var _this = this;
      if (!this.enabled)
         return;
      this.clicks++;
      if (this.clicks === 2) {
         this.clicks = 0;
         if (typeof this.doubleClickTimeout === "number") {
            clearTimeout(this.doubleClickTimeout);
            this.doubleClickTimeout = null;
         }
         return this.handleDoubleClick(e);
      }
      setTimeout(function () {
         _this.clicks = 0;
         _this.doubleClickTimeout = null;
      }, DOUBLE_CLICK_TIMEOUT);
      // NOTE: this is here to prevent click events on drag
      if (this.draggedEvents < DRAGGED_EVENTS_TOLERANCE)
         this.emit("click", (0, captor_1.getMouseCoords)(e, this.container));
   };
   MouseCaptor.prototype.handleRightClick = function (e) {
      if (!this.enabled)
         return;
      this.emit("rightClick", (0, captor_1.getMouseCoords)(e, this.container));
   };
   MouseCaptor.prototype.handleDoubleClick = function (e) {
      if (!this.enabled)
         return;
      e.preventDefault();
      e.stopPropagation();
      var mouseCoords = (0, captor_1.getMouseCoords)(e, this.container);
      this.emit("doubleClick", mouseCoords);
      if (mouseCoords.sigmaDefaultPrevented)
         return;
      // default behavior
      var camera = this.renderer.getCamera();
      var newRatio = camera.getBoundedRatio(camera.getState().ratio / DOUBLE_CLICK_ZOOMING_RATIO);
      camera.animate(this.renderer.getViewportZoomedState((0, captor_1.getPosition)(e, this.container), newRatio), {
         easing: "quadraticInOut",
         duration: DOUBLE_CLICK_ZOOMING_DURATION,
      });
   };
   MouseCaptor.prototype.handleDown = function (e) {
      if (!this.enabled)
         return;
      // We only start dragging on left button
      if (e.button === 0) {
         this.startCameraState = this.renderer.getCamera().getState();
         var _a = (0, captor_1.getPosition)(e, this.container), x = _a.x, y = _a.y;
         this.lastMouseX = x;
         this.lastMouseY = y;
         this.draggedEvents = 0;
         this.downStartTime = Date.now();
         this.isMouseDown = true;
      }
      this.emit("mousedown", (0, captor_1.getMouseCoords)(e, this.container));
   };
   MouseCaptor.prototype.handleUp = function (e) {
      var _this = this;
      if (!this.enabled || !this.isMouseDown)
         return;
      var camera = this.renderer.getCamera();
      this.isMouseDown = false;
      if (typeof this.movingTimeout === "number") {
         clearTimeout(this.movingTimeout);
         this.movingTimeout = null;
      }
      var _a = (0, captor_1.getPosition)(e, this.container), x = _a.x, y = _a.y;
      var cameraState = camera.getState(), previousCameraState = camera.getPreviousState() || { x: 0, y: 0 };
      if (this.isMoving) {
         camera.animate({
            x: cameraState.x + MOUSE_INERTIA_RATIO * (cameraState.x - previousCameraState.x),
            y: cameraState.y + MOUSE_INERTIA_RATIO * (cameraState.y - previousCameraState.y),
         }, {
            duration: MOUSE_INERTIA_DURATION,
            easing: "quadraticOut",
         });
      }
      else if (this.lastMouseX !== x || this.lastMouseY !== y) {
         camera.setState({
            x: cameraState.x,
            y: cameraState.y,
         });
      }
      this.isMoving = false;
      setTimeout(function () {
         _this.draggedEvents = 0;
         // NOTE: this refresh is here to make sure `hideEdgesOnMove` can work
         // when someone releases camera pan drag after having stopped moving.
         // See commit: https://github.com/jacomyal/sigma.js/commit/cfd9197f70319109db6b675dd7c82be493ca95a2
         // See also issue: https://github.com/jacomyal/sigma.js/issues/1290
         // It could be possible to render instead of scheduling a refresh but for
         // now it seems good enough.
         _this.renderer.refresh();
      }, 0);
      this.emit("mouseup", (0, captor_1.getMouseCoords)(e, this.container));
   };
   MouseCaptor.prototype.handleMove = function (e) {
      var _this = this;
      if (!this.enabled)
         return;
      var mouseCoords = (0, captor_1.getMouseCoords)(e, this.container);
      // Always trigger a "mousemovebody" event, so that it is possible to develop
      // a drag-and-drop effect that works even when the mouse is out of the
      // container:
      this.emit("mousemovebody", mouseCoords);
      // Only trigger the "mousemove" event when the mouse is actually hovering
      // the container, to avoid weirdly hovering nodes and/or edges when the
      // mouse is not hover the container:
      if (e.target === this.container) {
         this.emit("mousemove", mouseCoords);
      }
      if (mouseCoords.sigmaDefaultPrevented)
         return;
      // Handle the case when "isMouseDown" all the time, to allow dragging the
      // stage while the mouse is not hover the container:
      if (this.isMouseDown) {
         this.isMoving = true;
         this.draggedEvents++;
         if (typeof this.movingTimeout === "number") {
            clearTimeout(this.movingTimeout);
         }
         this.movingTimeout = window.setTimeout(function () {
            _this.movingTimeout = null;
            _this.isMoving = false;
         }, DRAG_TIMEOUT);
         var camera = this.renderer.getCamera();
         var _a = (0, captor_1.getPosition)(e, this.container), eX = _a.x, eY = _a.y;
         var lastMouse = this.renderer.viewportToFramedGraph({
            x: this.lastMouseX,
            y: this.lastMouseY,
         });
         var mouse = this.renderer.viewportToFramedGraph({ x: eX, y: eY });
         var offsetX = lastMouse.x - mouse.x, offsetY = lastMouse.y - mouse.y;
         var cameraState = camera.getState();
         var x = cameraState.x + offsetX, y = cameraState.y + offsetY;
         camera.setState({ x: x, y: y });
         this.lastMouseX = eX;
         this.lastMouseY = eY;
         e.preventDefault();
         e.stopPropagation();
      }
   };
   MouseCaptor.prototype.handleWheel = function (e) {
      var _this = this;
      if (!this.enabled)
         return;
      e.preventDefault();
      e.stopPropagation();
      var delta = (0, captor_1.getWheelDelta)(e);
      if (!delta)
         return;
      var wheelCoords = (0, captor_1.getWheelCoords)(e, this.container);
      this.emit("wheel", wheelCoords);
      if (wheelCoords.sigmaDefaultPrevented)
         return;
      // Default behavior
      var ratioDiff = delta > 0 ? 1 / ZOOMING_RATIO : ZOOMING_RATIO;
      var camera = this.renderer.getCamera();
      var newRatio = camera.getBoundedRatio(camera.getState().ratio * ratioDiff);
      var wheelDirection = delta > 0 ? 1 : -1;
      var now = Date.now();
      // Cancel events that are too close too each other and in the same direction:
      if (this.currentWheelDirection === wheelDirection &&
         this.lastWheelTriggerTime &&
         now - this.lastWheelTriggerTime < MOUSE_ZOOM_DURATION / 5) {
         return;
      }
      camera.animate(this.renderer.getViewportZoomedState((0, captor_1.getPosition)(e, this.container), newRatio), {
         easing: "quadraticOut",
         duration: MOUSE_ZOOM_DURATION,
      }, function () {
         _this.currentWheelDirection = 0;
      });
      this.currentWheelDirection = wheelDirection;
      this.lastWheelTriggerTime = now;
   };
   MouseCaptor.prototype.handleOut = function () {
      // TODO: dispatch event
   };
   return MouseCaptor;
}(captor_1.default));
exports["default"] = MouseCaptor;


/***/ }),
/* 13 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __assign = (this && this.__assign) || function () {
   __assign = Object.assign || function(t) {
      for (var s, i = 1, n = arguments.length; i < n; i++) {
         s = arguments[i];
         for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
            t[p] = s[p];
      }
      return t;
   };
   return __assign.apply(this, arguments);
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.getWheelDelta = exports.getTouchCoords = exports.getTouchesArray = exports.getWheelCoords = exports.getMouseCoords = exports.getPosition = void 0;
/**
 * Sigma.js Captor Class
 * ======================
 * @module
 */
var types_1 = __webpack_require__(10);
/**
 * Captor utils functions
 * ======================
 */
/**
 * Extract the local X and Y coordinates from a mouse event or touch object. If
 * a DOM element is given, it uses this element's offset to compute the position
 * (this allows using events that are not bound to the container itself and
 * still have a proper position).
 *
 * @param  {event}      e - A mouse event or touch object.
 * @param  {HTMLElement} dom - A DOM element to compute offset relatively to.
 * @return {number}     The local Y value of the mouse.
 */
function getPosition(e, dom) {
   var bbox = dom.getBoundingClientRect();
   return {
      x: e.clientX - bbox.left,
      y: e.clientY - bbox.top,
   };
}
exports.getPosition = getPosition;
/**
 * Convert mouse coords to sigma coords.
 *
 * @param  {event}      e   - A mouse event or touch object.
 * @param  {HTMLElement} dom - A DOM element to compute offset relatively to.
 * @return {object}
 */
function getMouseCoords(e, dom) {
   var res = __assign(__assign({}, getPosition(e, dom)), { sigmaDefaultPrevented: false, preventSigmaDefault: function () {
         res.sigmaDefaultPrevented = true;
      }, original: e });
   return res;
}
exports.getMouseCoords = getMouseCoords;
/**
 * Convert mouse wheel event coords to sigma coords.
 *
 * @param  {event}      e   - A wheel mouse event.
 * @param  {HTMLElement} dom - A DOM element to compute offset relatively to.
 * @return {object}
 */
function getWheelCoords(e, dom) {
   return __assign(__assign({}, getMouseCoords(e, dom)), { delta: getWheelDelta(e) });
}
exports.getWheelCoords = getWheelCoords;
var MAX_TOUCHES = 2;
function getTouchesArray(touches) {
   var arr = [];
   for (var i = 0, l = Math.min(touches.length, MAX_TOUCHES); i < l; i++)
      arr.push(touches[i]);
   return arr;
}
exports.getTouchesArray = getTouchesArray;
/**
 * Convert touch coords to sigma coords.
 *
 * @param  {event}      e   - A touch event.
 * @param  {HTMLElement} dom - A DOM element to compute offset relatively to.
 * @return {object}
 */
function getTouchCoords(e, dom) {
   return {
      touches: getTouchesArray(e.touches).map(function (touch) { return getPosition(touch, dom); }),
      original: e,
   };
}
exports.getTouchCoords = getTouchCoords;
/**
 * Extract the wheel delta from a mouse event or touch object.
 *
 * @param  {event}  e - A mouse event or touch object.
 * @return {number}    The wheel delta of the mouse.
 */
function getWheelDelta(e) {
   // TODO: check those ratios again to ensure a clean Chrome/Firefox compat
   if (typeof e.deltaY !== "undefined")
      return (e.deltaY * -3) / 360;
   if (typeof e.detail !== "undefined")
      return e.detail / -9;
   throw new Error("Captor: could not extract delta from event.");
}
exports.getWheelDelta = getWheelDelta;
/**
 * Abstract class representing a captor like the user's mouse or touch controls.
 */
var Captor = /** @class */ (function (_super) {
   __extends(Captor, _super);
   function Captor(container, renderer) {
      var _this = _super.call(this) || this;
      // Properties
      _this.container = container;
      _this.renderer = renderer;
      return _this;
   }
   return Captor;
}(types_1.TypedEventEmitter));
exports["default"] = Captor;


/***/ }),
/* 14 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.rectangleCollidesWithQuad = exports.squareCollidesWithQuad = exports.getCircumscribedAlignedRectangle = exports.isRectangleAligned = void 0;
/**
 * Sigma.js Quad Tree Class
 * =========================
 *
 * Class implementing the quad tree data structure used to solve hovers and
 * determine which elements are currently in the scope of the camera so that
 * we don't waste time rendering things the user cannot see anyway.
 * @module
 */
/* eslint no-nested-ternary: 0 */
/* eslint no-constant-condition: 0 */
var extend_1 = __importDefault(__webpack_require__(2));
/**
 * Notes:
 *
 *   - a square can be represented as topleft + width, saying for the quad blocks,
 *    to reduce overall memory usage (which is already pretty low).
 *   - this implementation of a quadtree is often called a MX-CIF quadtree.
 *   - we could explore spatial hashing (hilbert quadtrees, notably).
 */
/**
 * Constants.
 *
 * Note that since we are representing a static 4-ary tree, the indices of the
 * quadrants are the following:
 *   - TOP_LEFT:    4i + b
 *   - TOP_RIGHT:   4i + 2b
 *   - BOTTOM_LEFT:  4i + 3b
 *   - BOTTOM_RIGHT: 4i + 4b
 */
var BLOCKS = 4;
var MAX_LEVEL = 5;
// Outside block is max block index + 1, i.e.:
// BLOCKS * ((4 * (4 ** MAX_LEVEL) - 1) / 3)
var OUTSIDE_BLOCK = 5460;
var X_OFFSET = 0;
var Y_OFFSET = 1;
var WIDTH_OFFSET = 2;
var HEIGHT_OFFSET = 3;
var TOP_LEFT = 1;
var TOP_RIGHT = 2;
var BOTTOM_LEFT = 3;
var BOTTOM_RIGHT = 4;
var hasWarnedTooMuchOutside = false;
/**
 * Geometry helpers.
 */
/**
 * Function returning whether the given rectangle is axis-aligned.
 *
 * @param  {Rectangle} rect
 * @return {boolean}
 */
function isRectangleAligned(rect) {
   return rect.x1 === rect.x2 || rect.y1 === rect.y2;
}
exports.isRectangleAligned = isRectangleAligned;
/**
 * Function returning the smallest rectangle that contains the given rectangle, and that is aligned with the axis.
 *
 * @param {Rectangle} rect
 * @return {Rectangle}
 */
function getCircumscribedAlignedRectangle(rect) {
   var width = Math.sqrt(Math.pow(rect.x2 - rect.x1, 2) + Math.pow(rect.y2 - rect.y1, 2));
   var heightVector = {
      x: ((rect.y1 - rect.y2) * rect.height) / width,
      y: ((rect.x2 - rect.x1) * rect.height) / width,
   };
   // Compute all corners:
   var tl = { x: rect.x1, y: rect.y1 };
   var tr = { x: rect.x2, y: rect.y2 };
   var bl = {
      x: rect.x1 + heightVector.x,
      y: rect.y1 + heightVector.y,
   };
   var br = {
      x: rect.x2 + heightVector.x,
      y: rect.y2 + heightVector.y,
   };
   var xL = Math.min(tl.x, tr.x, bl.x, br.x);
   var xR = Math.max(tl.x, tr.x, bl.x, br.x);
   var yT = Math.min(tl.y, tr.y, bl.y, br.y);
   var yB = Math.max(tl.y, tr.y, bl.y, br.y);
   return {
      x1: xL,
      y1: yT,
      x2: xR,
      y2: yT,
      height: yB - yT,
   };
}
exports.getCircumscribedAlignedRectangle = getCircumscribedAlignedRectangle;
/**
 *
 * @param x1
 * @param y1
 * @param w
 * @param qx
 * @param qy
 * @param qw
 * @param qh
 */
function squareCollidesWithQuad(x1, y1, w, qx, qy, qw, qh) {
   return x1 < qx + qw && x1 + w > qx && y1 < qy + qh && y1 + w > qy;
}
exports.squareCollidesWithQuad = squareCollidesWithQuad;
function rectangleCollidesWithQuad(x1, y1, w, h, qx, qy, qw, qh) {
   return x1 < qx + qw && x1 + w > qx && y1 < qy + qh && y1 + h > qy;
}
exports.rectangleCollidesWithQuad = rectangleCollidesWithQuad;
function pointIsInQuad(x, y, qx, qy, qw, qh) {
   var xmp = qx + qw / 2, ymp = qy + qh / 2, top = y < ymp, left = x < xmp;
   return top ? (left ? TOP_LEFT : TOP_RIGHT) : left ? BOTTOM_LEFT : BOTTOM_RIGHT;
}
/**
 * Helper functions that are not bound to the class so an external user
 * cannot mess with them.
 */
function buildQuadrants(maxLevel, data) {
   // [block, level]
   var stack = [0, 0];
   while (stack.length) {
      var level = stack.pop(), block = stack.pop();
      var topLeftBlock = 4 * block + BLOCKS, topRightBlock = 4 * block + 2 * BLOCKS, bottomLeftBlock = 4 * block + 3 * BLOCKS, bottomRightBlock = 4 * block + 4 * BLOCKS;
      var x = data[block + X_OFFSET], y = data[block + Y_OFFSET], width = data[block + WIDTH_OFFSET], height = data[block + HEIGHT_OFFSET], hw = width / 2, hh = height / 2;
      data[topLeftBlock + X_OFFSET] = x;
      data[topLeftBlock + Y_OFFSET] = y;
      data[topLeftBlock + WIDTH_OFFSET] = hw;
      data[topLeftBlock + HEIGHT_OFFSET] = hh;
      data[topRightBlock + X_OFFSET] = x + hw;
      data[topRightBlock + Y_OFFSET] = y;
      data[topRightBlock + WIDTH_OFFSET] = hw;
      data[topRightBlock + HEIGHT_OFFSET] = hh;
      data[bottomLeftBlock + X_OFFSET] = x;
      data[bottomLeftBlock + Y_OFFSET] = y + hh;
      data[bottomLeftBlock + WIDTH_OFFSET] = hw;
      data[bottomLeftBlock + HEIGHT_OFFSET] = hh;
      data[bottomRightBlock + X_OFFSET] = x + hw;
      data[bottomRightBlock + Y_OFFSET] = y + hh;
      data[bottomRightBlock + WIDTH_OFFSET] = hw;
      data[bottomRightBlock + HEIGHT_OFFSET] = hh;
      if (level < maxLevel - 1) {
         stack.push(bottomRightBlock, level + 1);
         stack.push(bottomLeftBlock, level + 1);
         stack.push(topRightBlock, level + 1);
         stack.push(topLeftBlock, level + 1);
      }
   }
}
function insertNode(maxLevel, data, containers, key, x, y, size) {
   var x1 = x - size, y1 = y - size, w = size * 2;
   var level = 0, block = 0;
   while (true) {
      // If we reached max level
      if (level >= maxLevel) {
         containers[block] = containers[block] || [];
         containers[block].push(key);
         return;
      }
      var topLeftBlock = 4 * block + BLOCKS, topRightBlock = 4 * block + 2 * BLOCKS, bottomLeftBlock = 4 * block + 3 * BLOCKS, bottomRightBlock = 4 * block + 4 * BLOCKS;
      var collidingWithTopLeft = squareCollidesWithQuad(x1, y1, w, data[topLeftBlock + X_OFFSET], data[topLeftBlock + Y_OFFSET], data[topLeftBlock + WIDTH_OFFSET], data[topLeftBlock + HEIGHT_OFFSET]);
      var collidingWithTopRight = squareCollidesWithQuad(x1, y1, w, data[topRightBlock + X_OFFSET], data[topRightBlock + Y_OFFSET], data[topRightBlock + WIDTH_OFFSET], data[topRightBlock + HEIGHT_OFFSET]);
      var collidingWithBottomLeft = squareCollidesWithQuad(x1, y1, w, data[bottomLeftBlock + X_OFFSET], data[bottomLeftBlock + Y_OFFSET], data[bottomLeftBlock + WIDTH_OFFSET], data[bottomLeftBlock + HEIGHT_OFFSET]);
      var collidingWithBottomRight = squareCollidesWithQuad(x1, y1, w, data[bottomRightBlock + X_OFFSET], data[bottomRightBlock + Y_OFFSET], data[bottomRightBlock + WIDTH_OFFSET], data[bottomRightBlock + HEIGHT_OFFSET]);
      var collisions = [
         collidingWithTopLeft,
         collidingWithTopRight,
         collidingWithBottomLeft,
         collidingWithBottomRight,
      ].reduce(function (acc, current) {
         if (current)
            return acc + 1;
         else
            return acc;
      }, 0);
      // If we have no collision at root level, inject node in the outside block
      if (collisions === 0 && level === 0) {
         containers[OUTSIDE_BLOCK].push(key);
         if (!hasWarnedTooMuchOutside && containers[OUTSIDE_BLOCK].length >= 5) {
            hasWarnedTooMuchOutside = true;
            console.warn("sigma/quadtree.insertNode: At least 5 nodes are outside the global quadtree zone. " +
               "You might have a problem with the normalization function or the custom bounding box.");
         }
         return;
      }
      // If we don't have at least a collision but deeper, there is an issue
      if (collisions === 0)
         throw new Error("sigma/quadtree.insertNode: no collision (level: ".concat(level, ", key: ").concat(key, ", x: ").concat(x, ", y: ").concat(y, ", size: ").concat(size, ")."));
      // If we have 3 collisions, we have a geometry problem obviously
      if (collisions === 3)
         throw new Error("sigma/quadtree.insertNode: 3 impossible collisions (level: ".concat(level, ", key: ").concat(key, ", x: ").concat(x, ", y: ").concat(y, ", size: ").concat(size, ")."));
      // If we have more that one collision, we stop here and store the node
      // in the relevant containers
      if (collisions > 1) {
         containers[block] = containers[block] || [];
         containers[block].push(key);
         return;
      }
      else {
         level++;
      }
      // Else we recurse into the correct quads
      if (collidingWithTopLeft)
         block = topLeftBlock;
      if (collidingWithTopRight)
         block = topRightBlock;
      if (collidingWithBottomLeft)
         block = bottomLeftBlock;
      if (collidingWithBottomRight)
         block = bottomRightBlock;
   }
}
function getNodesInAxisAlignedRectangleArea(maxLevel, data, containers, x1, y1, w, h) {
   // [block, level]
   var stack = [0, 0];
   var collectedNodes = [];
   var container;
   while (stack.length) {
      var level = stack.pop(), block = stack.pop();
      // Collecting nodes
      container = containers[block];
      if (container)
         (0, extend_1.default)(collectedNodes, container);
      // If we reached max level
      if (level >= maxLevel)
         continue;
      var topLeftBlock = 4 * block + BLOCKS, topRightBlock = 4 * block + 2 * BLOCKS, bottomLeftBlock = 4 * block + 3 * BLOCKS, bottomRightBlock = 4 * block + 4 * BLOCKS;
      var collidingWithTopLeft = rectangleCollidesWithQuad(x1, y1, w, h, data[topLeftBlock + X_OFFSET], data[topLeftBlock + Y_OFFSET], data[topLeftBlock + WIDTH_OFFSET], data[topLeftBlock + HEIGHT_OFFSET]);
      var collidingWithTopRight = rectangleCollidesWithQuad(x1, y1, w, h, data[topRightBlock + X_OFFSET], data[topRightBlock + Y_OFFSET], data[topRightBlock + WIDTH_OFFSET], data[topRightBlock + HEIGHT_OFFSET]);
      var collidingWithBottomLeft = rectangleCollidesWithQuad(x1, y1, w, h, data[bottomLeftBlock + X_OFFSET], data[bottomLeftBlock + Y_OFFSET], data[bottomLeftBlock + WIDTH_OFFSET], data[bottomLeftBlock + HEIGHT_OFFSET]);
      var collidingWithBottomRight = rectangleCollidesWithQuad(x1, y1, w, h, data[bottomRightBlock + X_OFFSET], data[bottomRightBlock + Y_OFFSET], data[bottomRightBlock + WIDTH_OFFSET], data[bottomRightBlock + HEIGHT_OFFSET]);
      if (collidingWithTopLeft)
         stack.push(topLeftBlock, level + 1);
      if (collidingWithTopRight)
         stack.push(topRightBlock, level + 1);
      if (collidingWithBottomLeft)
         stack.push(bottomLeftBlock, level + 1);
      if (collidingWithBottomRight)
         stack.push(bottomRightBlock, level + 1);
   }
   return collectedNodes;
}
/**
 * QuadTree class.
 *
 * @constructor
 * @param {object} boundaries - The graph boundaries.
 */
var QuadTree = /** @class */ (function () {
   function QuadTree(params) {
      var _a;
      if (params === void 0) { params = {}; }
      this.containers = (_a = {}, _a[OUTSIDE_BLOCK] = [], _a);
      this.cache = null;
      this.lastRectangle = null;
      // Allocating the underlying byte array
      var L = Math.pow(4, MAX_LEVEL);
      this.data = new Float32Array(BLOCKS * ((4 * L - 1) / 3));
      if (params.boundaries)
         this.resize(params.boundaries);
      else
         this.resize({
            x: 0,
            y: 0,
            width: 1,
            height: 1,
         });
   }
   QuadTree.prototype.add = function (key, x, y, size) {
      insertNode(MAX_LEVEL, this.data, this.containers, key, x, y, size);
      return this;
   };
   QuadTree.prototype.resize = function (boundaries) {
      this.clear();
      // Building the quadrants
      this.data[X_OFFSET] = boundaries.x;
      this.data[Y_OFFSET] = boundaries.y;
      this.data[WIDTH_OFFSET] = boundaries.width;
      this.data[HEIGHT_OFFSET] = boundaries.height;
      buildQuadrants(MAX_LEVEL, this.data);
   };
   QuadTree.prototype.clear = function () {
      var _a;
      this.containers = (_a = {}, _a[OUTSIDE_BLOCK] = [], _a);
      return this;
   };
   QuadTree.prototype.point = function (x, y) {
      var nodes = this.containers[OUTSIDE_BLOCK].slice();
      var block = 0, level = 0;
      do {
         if (this.containers[block])
            (0, extend_1.default)(nodes, this.containers[block]);
         var quad = pointIsInQuad(x, y, this.data[block + X_OFFSET], this.data[block + Y_OFFSET], this.data[block + WIDTH_OFFSET], this.data[block + HEIGHT_OFFSET]);
         block = 4 * block + quad * BLOCKS;
         level++;
      } while (level <= MAX_LEVEL);
      return nodes;
   };
   QuadTree.prototype.rectangle = function (x1, y1, x2, y2, height) {
      var lr = this.lastRectangle;
      if (lr && x1 === lr.x1 && x2 === lr.x2 && y1 === lr.y1 && y2 === lr.y2 && height === lr.height) {
         return this.cache;
      }
      this.lastRectangle = {
         x1: x1,
         y1: y1,
         x2: x2,
         y2: y2,
         height: height,
      };
      // If the rectangle is shifted, we use the smallest aligned rectangle that contains the shifted one:
      if (!isRectangleAligned(this.lastRectangle))
         this.lastRectangle = getCircumscribedAlignedRectangle(this.lastRectangle);
      this.cache = getNodesInAxisAlignedRectangleArea(MAX_LEVEL, this.data, this.containers, x1, y1, Math.abs(x1 - x2) || Math.abs(y1 - y2), height);
      // Add all the nodes in the outside block, since they might be relevant, and since they should be very few:
      (0, extend_1.default)(this.cache, this.containers[OUTSIDE_BLOCK]);
      return this.cache;
   };
   return QuadTree;
}());
exports["default"] = QuadTree;


/***/ }),
/* 15 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.edgeLabelsToDisplayFromNodes = exports.LabelGrid = void 0;
/**
 * Class representing a single candidate for the label grid selection.
 *
 * It also describes a deterministic way to compare two candidates to assess
 * which one is better.
 */
var LabelCandidate = /** @class */ (function () {
   function LabelCandidate(key, size) {
      this.key = key;
      this.size = size;
   }
   LabelCandidate.compare = function (first, second) {
      // First we compare by size
      if (first.size > second.size)
         return -1;
      if (first.size < second.size)
         return 1;
      // Then since no two nodes can have the same key, we use it to
      // deterministically tie-break by key
      if (first.key > second.key)
         return 1;
      // NOTE: this comparator cannot return 0
      return -1;
   };
   return LabelCandidate;
}());
/**
 * Class representing a 2D spatial grid divided into constant-size cells.
 */
var LabelGrid = /** @class */ (function () {
   function LabelGrid() {
      this.width = 0;
      this.height = 0;
      this.cellSize = 0;
      this.columns = 0;
      this.rows = 0;
      this.cells = {};
   }
   LabelGrid.prototype.resizeAndClear = function (dimensions, cellSize) {
      this.width = dimensions.width;
      this.height = dimensions.height;
      this.cellSize = cellSize;
      this.columns = Math.ceil(dimensions.width / cellSize);
      this.rows = Math.ceil(dimensions.height / cellSize);
      this.cells = {};
   };
   LabelGrid.prototype.getIndex = function (pos) {
      var xIndex = Math.floor(pos.x / this.cellSize);
      var yIndex = Math.floor(pos.y / this.cellSize);
      return yIndex * this.columns + xIndex;
   };
   LabelGrid.prototype.add = function (key, size, pos) {
      var candidate = new LabelCandidate(key, size);
      var index = this.getIndex(pos);
      var cell = this.cells[index];
      if (!cell) {
         cell = [];
         this.cells[index] = cell;
      }
      cell.push(candidate);
   };
   LabelGrid.prototype.organize = function () {
      for (var k in this.cells) {
         var cell = this.cells[k];
         cell.sort(LabelCandidate.compare);
      }
   };
   LabelGrid.prototype.getLabelsToDisplay = function (ratio, density) {
      // TODO: work on visible nodes to optimize? ^ -> threshold outside so that memoization works?
      // TODO: adjust threshold lower, but increase cells a bit?
      // TODO: hunt for geom issue in disguise
      // TODO: memoize while ratio does not move. method to force recompute
      var cellArea = this.cellSize * this.cellSize;
      var scaledCellArea = cellArea / ratio / ratio;
      var scaledDensity = (scaledCellArea * density) / cellArea;
      var labelsToDisplayPerCell = Math.ceil(scaledDensity);
      var labels = [];
      for (var k in this.cells) {
         var cell = this.cells[k];
         for (var i = 0; i < Math.min(labelsToDisplayPerCell, cell.length); i++) {
            labels.push(cell[i].key);
         }
      }
      return labels;
   };
   return LabelGrid;
}());
exports.LabelGrid = LabelGrid;
/**
 * Label heuristic selecting edge labels to display, based on displayed node
 * labels
 *
 * @param  {object} params             - Parameters:
 * @param  {Set}     displayedNodeLabels  - Currently displayed node labels.
 * @param  {Set}     highlightedNodes    - Highlighted nodes.
 * @param  {Graph}   graph            - The rendered graph.
 * @param  {string}   hoveredNode        - Hovered node (optional)
 * @return {Array}                   - The selected labels.
 */
function edgeLabelsToDisplayFromNodes(params) {
   var graph = params.graph, hoveredNode = params.hoveredNode, highlightedNodes = params.highlightedNodes, displayedNodeLabels = params.displayedNodeLabels;
   var worthyEdges = [];
   // TODO: the code below can be optimized using #.forEach and batching the code per adj
   // We should display an edge's label if:
   //   - Any of its extremities is highlighted or hovered
   //   - Both of its extremities has its label shown
   graph.forEachEdge(function (edge, _, source, target) {
      if (source === hoveredNode ||
         target === hoveredNode ||
         highlightedNodes.has(source) ||
         highlightedNodes.has(target) ||
         (displayedNodeLabels.has(source) && displayedNodeLabels.has(target))) {
         worthyEdges.push(edge);
      }
   });
   return worthyEdges;
}
exports.edgeLabelsToDisplayFromNodes = edgeLabelsToDisplayFromNodes;


/***/ }),
/* 16 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.resolveSettings = exports.validateSettings = exports.DEFAULT_EDGE_PROGRAM_CLASSES = exports.DEFAULT_NODE_PROGRAM_CLASSES = exports.DEFAULT_SETTINGS = void 0;
var utils_1 = __webpack_require__(5);
var label_1 = __importDefault(__webpack_require__(17));
var hover_1 = __importDefault(__webpack_require__(18));
var edge_label_1 = __importDefault(__webpack_require__(19));
var node_fast_1 = __importDefault(__webpack_require__(20));
var edge_1 = __importDefault(__webpack_require__(26));
var edge_arrow_1 = __importDefault(__webpack_require__(30));
exports.DEFAULT_SETTINGS = {
   // Performance
   hideEdgesOnMove: false,
   hideLabelsOnMove: false,
   renderLabels: true,
   renderEdgeLabels: false,
   enableEdgeClickEvents: false,
   enableEdgeWheelEvents: false,
   enableEdgeHoverEvents: false,
   // Component rendering
   defaultNodeColor: "#999",
   defaultNodeType: "circle",
   defaultEdgeColor: "#ccc",
   defaultEdgeType: "line",
   labelFont: "Arial",
   labelSize: 14,
   labelWeight: "normal",
   labelColor: { color: "#000" },
   edgeLabelFont: "Arial",
   edgeLabelSize: 14,
   edgeLabelWeight: "normal",
   edgeLabelColor: { attribute: "color" },
   stagePadding: 30,
   // Labels
   labelDensity: 1,
   labelGridCellSize: 100,
   labelRenderedSizeThreshold: 6,
   // Reducers
   nodeReducer: null,
   edgeReducer: null,
   // Features
   zIndex: false,
   minCameraRatio: null,
   maxCameraRatio: null,
   // Renderers
   labelRenderer: label_1.default,
   hoverRenderer: hover_1.default,
   edgeLabelRenderer: edge_label_1.default,
   // Lifecycle
   allowInvalidContainer: false,
   // Program classes
   nodeProgramClasses: {},
   nodeHoverProgramClasses: {},
   edgeProgramClasses: {},
};
exports.DEFAULT_NODE_PROGRAM_CLASSES = {
   circle: node_fast_1.default,
};
exports.DEFAULT_EDGE_PROGRAM_CLASSES = {
   arrow: edge_arrow_1.default,
   line: edge_1.default,
};
function validateSettings(settings) {
   if (typeof settings.labelDensity !== "number" || settings.labelDensity < 0) {
      throw new Error("Settings: invalid `labelDensity`. Expecting a positive number.");
   }
   var minCameraRatio = settings.minCameraRatio, maxCameraRatio = settings.maxCameraRatio;
   if (typeof minCameraRatio === "number" && typeof maxCameraRatio === "number" && maxCameraRatio < minCameraRatio) {
      throw new Error("Settings: invalid camera ratio boundaries. Expecting `maxCameraRatio` to be greater than `minCameraRatio`.");
   }
}
exports.validateSettings = validateSettings;
function resolveSettings(settings) {
   var resolvedSettings = (0, utils_1.assign)({}, exports.DEFAULT_SETTINGS, settings);
   resolvedSettings.nodeProgramClasses = (0, utils_1.assign)({}, exports.DEFAULT_NODE_PROGRAM_CLASSES, resolvedSettings.nodeProgramClasses);
   resolvedSettings.edgeProgramClasses = (0, utils_1.assign)({}, exports.DEFAULT_EDGE_PROGRAM_CLASSES, resolvedSettings.edgeProgramClasses);
   return resolvedSettings;
}
exports.resolveSettings = resolveSettings;


/***/ }),
/* 17 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
function drawLabel(context, data, settings) {
   if (!data.label)
      return;
   var size = settings.labelSize, font = settings.labelFont, weight = settings.labelWeight, color = settings.labelColor.attribute
      ? data[settings.labelColor.attribute] || settings.labelColor.color || "#000"
      : settings.labelColor.color;
   context.fillStyle = color;
   context.font = "".concat(weight, " ").concat(size, "px ").concat(font);
   context.fillText(data.label, data.x + data.size + 3, data.y + size / 3);
}
exports["default"] = drawLabel;


/***/ }),
/* 18 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var label_1 = __importDefault(__webpack_require__(17));
/**
 * Draw an hovered node.
 * - if there is no label => display a shadow on the node
 * - if the label box is bigger than node size => display a label box that contains the node with a shadow
 * - else node with shadow and the label box
 */
function drawHover(context, data, settings) {
   var size = settings.labelSize, font = settings.labelFont, weight = settings.labelWeight;
   context.font = "".concat(weight, " ").concat(size, "px ").concat(font);
   // Then we draw the label background
   context.fillStyle = "#FFF";
   context.shadowOffsetX = 0;
   context.shadowOffsetY = 0;
   context.shadowBlur = 8;
   context.shadowColor = "#000";
   var PADDING = 2;
   if (typeof data.label === "string") {
      var textWidth = context.measureText(data.label).width, boxWidth = Math.round(textWidth + 5), boxHeight = Math.round(size + 2 * PADDING), radius = Math.max(data.size, size / 2) + PADDING;
      var angleRadian = Math.asin(boxHeight / 2 / radius);
      var xDeltaCoord = Math.sqrt(Math.abs(Math.pow(radius, 2) - Math.pow(boxHeight / 2, 2)));
      context.beginPath();
      context.moveTo(data.x + xDeltaCoord, data.y + boxHeight / 2);
      context.lineTo(data.x + radius + boxWidth, data.y + boxHeight / 2);
      context.lineTo(data.x + radius + boxWidth, data.y - boxHeight / 2);
      context.lineTo(data.x + xDeltaCoord, data.y - boxHeight / 2);
      context.arc(data.x, data.y, radius, angleRadian, -angleRadian);
      context.closePath();
      context.fill();
   }
   else {
      context.beginPath();
      context.arc(data.x, data.y, data.size + PADDING, 0, Math.PI * 2);
      context.closePath();
      context.fill();
   }
   context.shadowOffsetX = 0;
   context.shadowOffsetY = 0;
   context.shadowBlur = 0;
   // And finally we draw the label
   (0, label_1.default)(context, data, settings);
}
exports["default"] = drawHover;


/***/ }),
/* 19 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
function drawEdgeLabel(context, edgeData, sourceData, targetData, settings) {
   var size = settings.edgeLabelSize, font = settings.edgeLabelFont, weight = settings.edgeLabelWeight, color = settings.edgeLabelColor.attribute
      ? edgeData[settings.edgeLabelColor.attribute] || settings.edgeLabelColor.color || "#000"
      : settings.edgeLabelColor.color;
   var label = edgeData.label;
   if (!label)
      return;
   context.fillStyle = color;
   context.font = "".concat(weight, " ").concat(size, "px ").concat(font);
   // Computing positions without considering nodes sizes:
   var sSize = sourceData.size;
   var tSize = targetData.size;
   var sx = sourceData.x;
   var sy = sourceData.y;
   var tx = targetData.x;
   var ty = targetData.y;
   var cx = (sx + tx) / 2;
   var cy = (sy + ty) / 2;
   var dx = tx - sx;
   var dy = ty - sy;
   var d = Math.sqrt(dx * dx + dy * dy);
   if (d < sSize + tSize)
      return;
   // Adding nodes sizes:
   sx += (dx * sSize) / d;
   sy += (dy * sSize) / d;
   tx -= (dx * tSize) / d;
   ty -= (dy * tSize) / d;
   cx = (sx + tx) / 2;
   cy = (sy + ty) / 2;
   dx = tx - sx;
   dy = ty - sy;
   d = Math.sqrt(dx * dx + dy * dy);
   // Handling ellipsis
   var textLength = context.measureText(label).width;
   if (textLength > d) {
      var ellipsis = "…";
      label = label + ellipsis;
      textLength = context.measureText(label).width;
      while (textLength > d && label.length > 1) {
         label = label.slice(0, -2) + ellipsis;
         textLength = context.measureText(label).width;
      }
      if (label.length < 4)
         return;
   }
   var angle;
   if (dx > 0) {
      if (dy > 0)
         angle = Math.acos(dx / d);
      else
         angle = Math.asin(dy / d);
   }
   else {
      if (dy > 0)
         angle = Math.acos(dx / d) + Math.PI;
      else
         angle = Math.asin(dx / d) + Math.PI / 2;
   }
   context.save();
   context.translate(cx, cy);
   context.rotate(angle);
   context.fillText(label, -textLength / 2, edgeData.size / 2 + size);
   context.restore();
}
exports["default"] = drawEdgeLabel;


/***/ }),
/* 20 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var utils_1 = __webpack_require__(5);
var node_fast_vert_glsl_1 = __importDefault(__webpack_require__(21));
var node_fast_frag_glsl_1 = __importDefault(__webpack_require__(22));
var node_1 = __webpack_require__(23);
var POINTS = 1, ATTRIBUTES = 4;
var NodeFastProgram = /** @class */ (function (_super) {
   __extends(NodeFastProgram, _super);
   function NodeFastProgram(gl) {
      var _this = _super.call(this, gl, node_fast_vert_glsl_1.default, node_fast_frag_glsl_1.default, POINTS, ATTRIBUTES) || this;
      _this.bind();
      return _this;
   }
   NodeFastProgram.prototype.process = function (data, hidden, offset) {
      var array = this.array;
      var i = offset * POINTS * ATTRIBUTES;
      if (hidden) {
         array[i++] = 0;
         array[i++] = 0;
         array[i++] = 0;
         array[i++] = 0;
         return;
      }
      var color = (0, utils_1.floatColor)(data.color);
      array[i++] = data.x;
      array[i++] = data.y;
      array[i++] = data.size;
      array[i] = color;
   };
   NodeFastProgram.prototype.render = function (params) {
      if (this.hasNothingToRender())
         return;
      var gl = this.gl;
      var program = this.program;
      gl.useProgram(program);
      gl.uniform1f(this.ratioLocation, 1 / Math.sqrt(params.ratio));
      gl.uniform1f(this.scaleLocation, params.scalingRatio);
      gl.uniformMatrix3fv(this.matrixLocation, false, params.matrix);
      gl.drawArrays(gl.POINTS, 0, this.array.length / ATTRIBUTES);
   };
   return NodeFastProgram;
}(node_1.AbstractNodeProgram));
exports["default"] = NodeFastProgram;


/***/ }),
/* 21 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("attribute vec2 a_position;\nattribute float a_size;\nattribute vec4 a_color;\n\nuniform float u_ratio;\nuniform float u_scale;\nuniform mat3 u_matrix;\n\nvarying vec4 v_color;\nvarying float v_border;\n\nconst float bias = 255.0 / 254.0;\n\nvoid main() {\n  gl_Position = vec4(\n   (u_matrix * vec3(a_position, 1)).xy,\n   0,\n   1\n  );\n\n  // Multiply the point size twice:\n  //  - x SCALING_RATIO to correct the canvas scaling\n  //  - x 2 to correct the formulae\n  gl_PointSize = a_size * u_ratio * u_scale * 2.0;\n\n  v_border = (1.0 / u_ratio) * (0.5 / a_size);\n\n  // Extract the color:\n  v_color = a_color;\n  v_color.a *= bias;\n}\n");

/***/ }),
/* 22 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("precision mediump float;\n\nvarying vec4 v_color;\nvarying float v_border;\n\nconst float radius = 0.5;\nconst vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);\n\nvoid main(void) {\n  vec2 m = gl_PointCoord - vec2(0.5, 0.5);\n  float dist = radius - length(m);\n\n  float t = 0.0;\n  if (dist > v_border)\n   t = 1.0;\n  else if (dist > 0.0)\n   t = dist / v_border;\n\n  gl_FragColor = mix(transparent, v_color, t);\n}\n");

/***/ }),
/* 23 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.createNodeCompoundProgram = exports.AbstractNodeProgram = void 0;
/**
 * Sigma.js WebGL Abstract Node Program
 * =====================================
 *
 * @module
 */
var program_1 = __webpack_require__(24);
/**
 * Node Program class.
 *
 * @constructor
 */
var AbstractNodeProgram = /** @class */ (function (_super) {
   __extends(AbstractNodeProgram, _super);
   function AbstractNodeProgram(gl, vertexShaderSource, fragmentShaderSource, points, attributes) {
      var _this = _super.call(this, gl, vertexShaderSource, fragmentShaderSource, points, attributes) || this;
      // Locations
      _this.positionLocation = gl.getAttribLocation(_this.program, "a_position");
      _this.sizeLocation = gl.getAttribLocation(_this.program, "a_size");
      _this.colorLocation = gl.getAttribLocation(_this.program, "a_color");
      // Uniform Location
      var matrixLocation = gl.getUniformLocation(_this.program, "u_matrix");
      if (matrixLocation === null)
         throw new Error("AbstractNodeProgram: error while getting matrixLocation");
      _this.matrixLocation = matrixLocation;
      var ratioLocation = gl.getUniformLocation(_this.program, "u_ratio");
      if (ratioLocation === null)
         throw new Error("AbstractNodeProgram: error while getting ratioLocation");
      _this.ratioLocation = ratioLocation;
      var scaleLocation = gl.getUniformLocation(_this.program, "u_scale");
      if (scaleLocation === null)
         throw new Error("AbstractNodeProgram: error while getting scaleLocation");
      _this.scaleLocation = scaleLocation;
      return _this;
   }
   AbstractNodeProgram.prototype.bind = function () {
      var gl = this.gl;
      gl.enableVertexAttribArray(this.positionLocation);
      gl.enableVertexAttribArray(this.sizeLocation);
      gl.enableVertexAttribArray(this.colorLocation);
      gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, this.attributes * Float32Array.BYTES_PER_ELEMENT, 0);
      gl.vertexAttribPointer(this.sizeLocation, 1, gl.FLOAT, false, this.attributes * Float32Array.BYTES_PER_ELEMENT, 8);
      gl.vertexAttribPointer(this.colorLocation, 4, gl.UNSIGNED_BYTE, true, this.attributes * Float32Array.BYTES_PER_ELEMENT, 12);
   };
   return AbstractNodeProgram;
}(program_1.AbstractProgram));
exports.AbstractNodeProgram = AbstractNodeProgram;
/**
 * Helper function combining two or more programs into a single compound one.
 * Note that this is more a quick & easy way to combine program than a really
 * performant option. More performant programs can be written entirely.
 *
 * @param  {array}   programClasses - Program classes to combine.
 * @return {function}
 */
function createNodeCompoundProgram(programClasses) {
   return /** @class */ (function () {
      function NodeCompoundProgram(gl, renderer) {
         this.programs = programClasses.map(function (ProgramClass) { return new ProgramClass(gl, renderer); });
      }
      NodeCompoundProgram.prototype.bufferData = function () {
         this.programs.forEach(function (program) { return program.bufferData(); });
      };
      NodeCompoundProgram.prototype.allocate = function (capacity) {
         this.programs.forEach(function (program) { return program.allocate(capacity); });
      };
      NodeCompoundProgram.prototype.bind = function () {
         // nothing todo, it's already done in each program constructor
      };
      NodeCompoundProgram.prototype.render = function (params) {
         this.programs.forEach(function (program) {
            program.bind();
            program.bufferData();
            program.render(params);
         });
      };
      NodeCompoundProgram.prototype.process = function (data, hidden, offset) {
         this.programs.forEach(function (program) { return program.process(data, hidden, offset); });
      };
      return NodeCompoundProgram;
   }());
}
exports.createNodeCompoundProgram = createNodeCompoundProgram;


/***/ }),
/* 24 */
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.AbstractProgram = void 0;
/**
 * Sigma.js WebGL Renderer Program
 * ================================
 *
 * Class representing a single WebGL program used by sigma's WebGL renderer.
 * @module
 */
var utils_1 = __webpack_require__(25);
/**
 * Abstract Program class.
 *
 * @constructor
 */
var AbstractProgram = /** @class */ (function () {
   function AbstractProgram(gl, vertexShaderSource, fragmentShaderSource, points, attributes) {
      this.array = new Float32Array();
      this.points = points;
      this.attributes = attributes;
      this.gl = gl;
      this.vertexShaderSource = vertexShaderSource;
      this.fragmentShaderSource = fragmentShaderSource;
      var buffer = gl.createBuffer();
      if (buffer === null)
         throw new Error("AbstractProgram: error while creating the buffer");
      this.buffer = buffer;
      gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
      this.vertexShader = (0, utils_1.loadVertexShader)(gl, this.vertexShaderSource);
      this.fragmentShader = (0, utils_1.loadFragmentShader)(gl, this.fragmentShaderSource);
      this.program = (0, utils_1.loadProgram)(gl, [this.vertexShader, this.fragmentShader]);
   }
   AbstractProgram.prototype.bufferData = function () {
      var gl = this.gl;
      gl.bufferData(gl.ARRAY_BUFFER, this.array, gl.DYNAMIC_DRAW);
   };
   AbstractProgram.prototype.allocate = function (capacity) {
      this.array = new Float32Array(this.points * this.attributes * capacity);
   };
   AbstractProgram.prototype.hasNothingToRender = function () {
      return this.array.length === 0;
   };
   return AbstractProgram;
}());
exports.AbstractProgram = AbstractProgram;


/***/ }),
/* 25 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

/**
 * Sigma.js Shader Utils
 * ======================
 *
 * Code used to load sigma's shaders.
 * @module
 */
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.loadProgram = exports.loadFragmentShader = exports.loadVertexShader = void 0;
/**
 * Function used to load a shader.
 */
function loadShader(type, gl, source) {
   var glType = type === "VERTEX" ? gl.VERTEX_SHADER : gl.FRAGMENT_SHADER;
   // Creating the shader
   var shader = gl.createShader(glType);
   if (shader === null) {
      throw new Error("loadShader: error while creating the shader");
   }
   // Loading source
   gl.shaderSource(shader, source);
   // Compiling the shader
   gl.compileShader(shader);
   // Retrieving compilation status
   var successfullyCompiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS);
   // Throwing if something went awry
   if (!successfullyCompiled) {
      var infoLog = gl.getShaderInfoLog(shader);
      gl.deleteShader(shader);
      throw new Error("loadShader: error while compiling the shader:\n".concat(infoLog, "\n").concat(source));
   }
   return shader;
}
function loadVertexShader(gl, source) {
   return loadShader("VERTEX", gl, source);
}
exports.loadVertexShader = loadVertexShader;
function loadFragmentShader(gl, source) {
   return loadShader("FRAGMENT", gl, source);
}
exports.loadFragmentShader = loadFragmentShader;
/**
 * Function used to load a program.
 */
function loadProgram(gl, shaders) {
   var program = gl.createProgram();
   if (program === null) {
      throw new Error("loadProgram: error while creating the program.");
   }
   var i, l;
   // Attaching the shaders
   for (i = 0, l = shaders.length; i < l; i++)
      gl.attachShader(program, shaders[i]);
   gl.linkProgram(program);
   // Checking status
   var successfullyLinked = gl.getProgramParameter(program, gl.LINK_STATUS);
   if (!successfullyLinked) {
      gl.deleteProgram(program);
      throw new Error("loadProgram: error while linking the program.");
   }
   return program;
}
exports.loadProgram = loadProgram;


/***/ }),
/* 26 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
/**
 * Sigma.js WebGL Renderer Edge Program
 * =====================================
 *
 * Program rendering edges as thick lines using four points translated
 * orthogonally from the source & target's centers by half thickness.
 *
 * Rendering two triangles by using only four points is made possible through
 * the use of indices.
 *
 * This method should be faster than the 6 points / 2 triangles approach and
 * should handle thickness better than with gl.LINES.
 *
 * This version of the shader balances geometry computation evenly between
 * the CPU & GPU (normals are computed on the CPU side).
 * @module
 */
var utils_1 = __webpack_require__(5);
var edge_vert_glsl_1 = __importDefault(__webpack_require__(27));
var edge_frag_glsl_1 = __importDefault(__webpack_require__(28));
var edge_1 = __webpack_require__(29);
var POINTS = 4, ATTRIBUTES = 5, STRIDE = POINTS * ATTRIBUTES;
var EdgeProgram = /** @class */ (function (_super) {
   __extends(EdgeProgram, _super);
   function EdgeProgram(gl) {
      var _this = _super.call(this, gl, edge_vert_glsl_1.default, edge_frag_glsl_1.default, POINTS, ATTRIBUTES) || this;
      // Initializing indices buffer
      var indicesBuffer = gl.createBuffer();
      if (indicesBuffer === null)
         throw new Error("EdgeProgram: error while creating indicesBuffer");
      _this.indicesBuffer = indicesBuffer;
      // Locations
      _this.positionLocation = gl.getAttribLocation(_this.program, "a_position");
      _this.colorLocation = gl.getAttribLocation(_this.program, "a_color");
      _this.normalLocation = gl.getAttribLocation(_this.program, "a_normal");
      var matrixLocation = gl.getUniformLocation(_this.program, "u_matrix");
      if (matrixLocation === null)
         throw new Error("EdgeProgram: error while getting matrixLocation");
      _this.matrixLocation = matrixLocation;
      var correctionRatioLocation = gl.getUniformLocation(_this.program, "u_correctionRatio");
      if (correctionRatioLocation === null)
         throw new Error("EdgeProgram: error while getting correctionRatioLocation");
      _this.correctionRatioLocation = correctionRatioLocation;
      var sqrtZoomRatioLocation = gl.getUniformLocation(_this.program, "u_sqrtZoomRatio");
      if (sqrtZoomRatioLocation === null)
         throw new Error("EdgeProgram: error while getting sqrtZoomRatioLocation");
      _this.sqrtZoomRatioLocation = sqrtZoomRatioLocation;
      // Enabling the OES_element_index_uint extension
      // NOTE: on older GPUs, this means that really large graphs won't
      // have all their edges rendered. But it seems that the
      // `OES_element_index_uint` is quite everywhere so we'll handle
      // the potential issue if it really arises.
      // NOTE: when using webgl2, the extension is enabled by default
      _this.canUse32BitsIndices = (0, utils_1.canUse32BitsIndices)(gl);
      _this.IndicesArray = _this.canUse32BitsIndices ? Uint32Array : Uint16Array;
      _this.indicesArray = new _this.IndicesArray();
      _this.indicesType = _this.canUse32BitsIndices ? gl.UNSIGNED_INT : gl.UNSIGNED_SHORT;
      _this.bind();
      return _this;
   }
   EdgeProgram.prototype.bind = function () {
      var gl = this.gl;
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);
      // Bindings
      gl.enableVertexAttribArray(this.positionLocation);
      gl.enableVertexAttribArray(this.normalLocation);
      gl.enableVertexAttribArray(this.colorLocation);
      gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 0);
      gl.vertexAttribPointer(this.normalLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 8);
      gl.vertexAttribPointer(this.colorLocation, 4, gl.UNSIGNED_BYTE, true, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 16);
   };
   EdgeProgram.prototype.computeIndices = function () {
      var l = this.array.length / ATTRIBUTES;
      var size = l + l / 2;
      var indices = new this.IndicesArray(size);
      for (var i = 0, c = 0; i < l; i += 4) {
         indices[c++] = i;
         indices[c++] = i + 1;
         indices[c++] = i + 2;
         indices[c++] = i + 2;
         indices[c++] = i + 1;
         indices[c++] = i + 3;
      }
      this.indicesArray = indices;
   };
   EdgeProgram.prototype.bufferData = function () {
      _super.prototype.bufferData.call(this);
      // Indices data
      var gl = this.gl;
      gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.indicesArray, gl.STATIC_DRAW);
   };
   EdgeProgram.prototype.process = function (sourceData, targetData, data, hidden, offset) {
      if (hidden) {
         for (var i_1 = offset * STRIDE, l = i_1 + STRIDE; i_1 < l; i_1++)
            this.array[i_1] = 0;
         return;
      }
      var thickness = data.size || 1, x1 = sourceData.x, y1 = sourceData.y, x2 = targetData.x, y2 = targetData.y, color = (0, utils_1.floatColor)(data.color);
      // Computing normals
      var dx = x2 - x1, dy = y2 - y1;
      var len = dx * dx + dy * dy, n1 = 0, n2 = 0;
      if (len) {
         len = 1 / Math.sqrt(len);
         n1 = -dy * len * thickness;
         n2 = dx * len * thickness;
      }
      var i = POINTS * ATTRIBUTES * offset;
      var array = this.array;
      // First point
      array[i++] = x1;
      array[i++] = y1;
      array[i++] = n1;
      array[i++] = n2;
      array[i++] = color;
      // First point flipped
      array[i++] = x1;
      array[i++] = y1;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = color;
      // Second point
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = n1;
      array[i++] = n2;
      array[i++] = color;
      // Second point flipped
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i] = color;
   };
   EdgeProgram.prototype.render = function (params) {
      if (this.hasNothingToRender())
         return;
      var gl = this.gl;
      var program = this.program;
      gl.useProgram(program);
      gl.uniformMatrix3fv(this.matrixLocation, false, params.matrix);
      gl.uniform1f(this.sqrtZoomRatioLocation, Math.sqrt(params.ratio));
      gl.uniform1f(this.correctionRatioLocation, params.correctionRatio);
      // Drawing:
      gl.drawElements(gl.TRIANGLES, this.indicesArray.length, this.indicesType, 0);
   };
   return EdgeProgram;
}(edge_1.AbstractEdgeProgram));
exports["default"] = EdgeProgram;


/***/ }),
/* 27 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("attribute vec4 a_color;\nattribute vec2 a_normal;\nattribute vec2 a_position;\n\nuniform mat3 u_matrix;\nuniform float u_sqrtZoomRatio;\nuniform float u_correctionRatio;\n\nvarying vec4 v_color;\nvarying vec2 v_normal;\nvarying float v_thickness;\n\nconst float minThickness = 1.7;\nconst float bias = 255.0 / 254.0;\n\nvoid main() {\n  float normalLength = length(a_normal);\n  vec2 unitNormal = a_normal / normalLength;\n\n  // We require edges to be at least `minThickness` pixels thick *on screen*\n  // (so we need to compensate the SQRT zoom ratio):\n  float pixelsThickness = max(normalLength, minThickness * u_sqrtZoomRatio);\n\n  // Then, we need to retrieve the normalized thickness of the edge in the WebGL\n  // referential (in a ([0, 1], [0, 1]) space), using our \"magic\" correction\n  // ratio:\n  float webGLThickness = pixelsThickness * u_correctionRatio;\n\n  // Finally, we adapt the edge thickness to the \"SQRT rule\" in sigma (so that\n  // items are not too big when zoomed in, and not too small when zoomed out).\n  // The exact computation should be `adapted = value * zoom / sqrt(zoom)`, but\n  // it's simpler like this:\n  float adaptedWebGLThickness = webGLThickness * u_sqrtZoomRatio;\n\n  // Here is the proper position of the vertex\n  gl_Position = vec4((u_matrix * vec3(a_position + unitNormal * adaptedWebGLThickness, 1)).xy, 0, 1);\n\n  // For the fragment shader though, we need a thickness that takes the \"magic\"\n  // correction ratio into account (as in webGLThickness), but so that the\n  // antialiasing effect does not depend on the zoom level. So here's yet\n  // another thickness version:\n  v_thickness = webGLThickness / u_sqrtZoomRatio;\n\n  v_normal = unitNormal;\n  v_color = a_color;\n  v_color.a *= bias;\n}\n");

/***/ }),
/* 28 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("precision mediump float;\n\nvarying vec4 v_color;\nvarying vec2 v_normal;\nvarying float v_thickness;\n\nconst float feather = 0.001;\nconst vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);\n\nvoid main(void) {\n  float dist = length(v_normal) * v_thickness;\n\n  float t = smoothstep(\n   v_thickness - feather,\n   v_thickness,\n   dist\n  );\n\n  gl_FragColor = mix(v_color, transparent, t);\n}\n");

/***/ }),
/* 29 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.createEdgeCompoundProgram = exports.AbstractEdgeProgram = void 0;
/**
 * Sigma.js WebGL Abstract Edge Program
 * =====================================
 *
 * @module
 */
var program_1 = __webpack_require__(24);
/**
 * Edge Program class.
 *
 * @constructor
 */
var AbstractEdgeProgram = /** @class */ (function (_super) {
   __extends(AbstractEdgeProgram, _super);
   function AbstractEdgeProgram(gl, vertexShaderSource, fragmentShaderSource, points, attributes) {
      return _super.call(this, gl, vertexShaderSource, fragmentShaderSource, points, attributes) || this;
   }
   return AbstractEdgeProgram;
}(program_1.AbstractProgram));
exports.AbstractEdgeProgram = AbstractEdgeProgram;
function createEdgeCompoundProgram(programClasses) {
   return /** @class */ (function () {
      function EdgeCompoundProgram(gl, renderer) {
         this.programs = programClasses.map(function (ProgramClass) { return new ProgramClass(gl, renderer); });
      }
      EdgeCompoundProgram.prototype.bufferData = function () {
         this.programs.forEach(function (program) { return program.bufferData(); });
      };
      EdgeCompoundProgram.prototype.allocate = function (capacity) {
         this.programs.forEach(function (program) { return program.allocate(capacity); });
      };
      EdgeCompoundProgram.prototype.bind = function () {
         // nothing todo, it's already done in each program constructor
      };
      EdgeCompoundProgram.prototype.computeIndices = function () {
         this.programs.forEach(function (program) { return program.computeIndices(); });
      };
      EdgeCompoundProgram.prototype.render = function (params) {
         this.programs.forEach(function (program) {
            program.bind();
            program.bufferData();
            program.render(params);
         });
      };
      EdgeCompoundProgram.prototype.process = function (sourceData, targetData, data, hidden, offset) {
         this.programs.forEach(function (program) { return program.process(sourceData, targetData, data, hidden, offset); });
      };
      return EdgeCompoundProgram;
   }());
}
exports.createEdgeCompoundProgram = createEdgeCompoundProgram;


/***/ }),
/* 30 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
/**
 * Sigma.js WebGL Renderer Edge Arrow Program
 * ===========================================
 *
 * Compound program rendering edges as an arrow from the source to the target.
 * @module
 */
var edge_1 = __webpack_require__(29);
var edge_arrowHead_1 = __importDefault(__webpack_require__(31));
var edge_clamped_1 = __importDefault(__webpack_require__(34));
var EdgeArrowProgram = (0, edge_1.createEdgeCompoundProgram)([edge_clamped_1.default, edge_arrowHead_1.default]);
exports["default"] = EdgeArrowProgram;


/***/ }),
/* 31 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var utils_1 = __webpack_require__(5);
var edge_arrowHead_vert_glsl_1 = __importDefault(__webpack_require__(32));
var edge_arrowHead_frag_glsl_1 = __importDefault(__webpack_require__(33));
var edge_1 = __webpack_require__(29);
var POINTS = 3, ATTRIBUTES = 9, STRIDE = POINTS * ATTRIBUTES;
var EdgeArrowHeadProgram = /** @class */ (function (_super) {
   __extends(EdgeArrowHeadProgram, _super);
   function EdgeArrowHeadProgram(gl) {
      var _this = _super.call(this, gl, edge_arrowHead_vert_glsl_1.default, edge_arrowHead_frag_glsl_1.default, POINTS, ATTRIBUTES) || this;
      // Locations
      _this.positionLocation = gl.getAttribLocation(_this.program, "a_position");
      _this.colorLocation = gl.getAttribLocation(_this.program, "a_color");
      _this.normalLocation = gl.getAttribLocation(_this.program, "a_normal");
      _this.radiusLocation = gl.getAttribLocation(_this.program, "a_radius");
      _this.barycentricLocation = gl.getAttribLocation(_this.program, "a_barycentric");
      // Uniform locations
      var matrixLocation = gl.getUniformLocation(_this.program, "u_matrix");
      if (matrixLocation === null)
         throw new Error("EdgeArrowHeadProgram: error while getting matrixLocation");
      _this.matrixLocation = matrixLocation;
      var sqrtZoomRatioLocation = gl.getUniformLocation(_this.program, "u_sqrtZoomRatio");
      if (sqrtZoomRatioLocation === null)
         throw new Error("EdgeArrowHeadProgram: error while getting sqrtZoomRatioLocation");
      _this.sqrtZoomRatioLocation = sqrtZoomRatioLocation;
      var correctionRatioLocation = gl.getUniformLocation(_this.program, "u_correctionRatio");
      if (correctionRatioLocation === null)
         throw new Error("EdgeArrowHeadProgram: error while getting correctionRatioLocation");
      _this.correctionRatioLocation = correctionRatioLocation;
      _this.bind();
      return _this;
   }
   EdgeArrowHeadProgram.prototype.bind = function () {
      var gl = this.gl;
      // Bindings
      gl.enableVertexAttribArray(this.positionLocation);
      gl.enableVertexAttribArray(this.normalLocation);
      gl.enableVertexAttribArray(this.radiusLocation);
      gl.enableVertexAttribArray(this.colorLocation);
      gl.enableVertexAttribArray(this.barycentricLocation);
      gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 0);
      gl.vertexAttribPointer(this.normalLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 8);
      gl.vertexAttribPointer(this.radiusLocation, 1, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 16);
      gl.vertexAttribPointer(this.colorLocation, 4, gl.UNSIGNED_BYTE, true, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 20);
      // TODO: maybe we can optimize here by packing this in a bit mask
      gl.vertexAttribPointer(this.barycentricLocation, 3, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 24);
   };
   EdgeArrowHeadProgram.prototype.computeIndices = function () {
      // nothing to do
   };
   EdgeArrowHeadProgram.prototype.process = function (sourceData, targetData, data, hidden, offset) {
      if (hidden) {
         for (var i_1 = offset * STRIDE, l = i_1 + STRIDE; i_1 < l; i_1++)
            this.array[i_1] = 0;
         return;
      }
      var thickness = data.size || 1, radius = targetData.size || 1, x1 = sourceData.x, y1 = sourceData.y, x2 = targetData.x, y2 = targetData.y, color = (0, utils_1.floatColor)(data.color);
      // Computing normals
      var dx = x2 - x1, dy = y2 - y1;
      var len = dx * dx + dy * dy, n1 = 0, n2 = 0;
      if (len) {
         len = 1 / Math.sqrt(len);
         n1 = -dy * len * thickness;
         n2 = dx * len * thickness;
      }
      var i = POINTS * ATTRIBUTES * offset;
      var array = this.array;
      // First point
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = radius;
      array[i++] = color;
      array[i++] = 1;
      array[i++] = 0;
      array[i++] = 0;
      // Second point
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = radius;
      array[i++] = color;
      array[i++] = 0;
      array[i++] = 1;
      array[i++] = 0;
      // Third point
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = radius;
      array[i++] = color;
      array[i++] = 0;
      array[i++] = 0;
      array[i] = 1;
   };
   EdgeArrowHeadProgram.prototype.render = function (params) {
      if (this.hasNothingToRender())
         return;
      var gl = this.gl;
      var program = this.program;
      gl.useProgram(program);
      // Binding uniforms
      gl.uniformMatrix3fv(this.matrixLocation, false, params.matrix);
      gl.uniform1f(this.sqrtZoomRatioLocation, Math.sqrt(params.ratio));
      gl.uniform1f(this.correctionRatioLocation, params.correctionRatio);
      // Drawing:
      gl.drawArrays(gl.TRIANGLES, 0, this.array.length / ATTRIBUTES);
   };
   return EdgeArrowHeadProgram;
}(edge_1.AbstractEdgeProgram));
exports["default"] = EdgeArrowHeadProgram;


/***/ }),
/* 32 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("attribute vec2 a_position;\nattribute vec2 a_normal;\nattribute float a_radius;\nattribute vec4 a_color;\nattribute vec3 a_barycentric;\n\nuniform mat3 u_matrix;\nuniform float u_sqrtZoomRatio;\nuniform float u_correctionRatio;\n\nvarying vec4 v_color;\n\nconst float minThickness = 1.7;\nconst float bias = 255.0 / 254.0;\nconst float arrowHeadWidthLengthRatio = 0.66;\nconst float arrowHeadLengthThicknessRatio = 2.5;\n\nvoid main() {\n  float normalLength = length(a_normal);\n  vec2 unitNormal = a_normal / normalLength;\n\n  // These first computations are taken from edge.vert.glsl and\n  // edge.clamped.vert.glsl. Please read it to get better comments on what's\n  // happening:\n  float pixelsThickness = max(normalLength, minThickness * u_sqrtZoomRatio);\n  float webGLThickness = pixelsThickness * u_correctionRatio;\n  float adaptedWebGLThickness = webGLThickness * u_sqrtZoomRatio;\n  float adaptedWebGLNodeRadius = a_radius * 2.0 * u_correctionRatio * u_sqrtZoomRatio;\n  float adaptedWebGLArrowHeadLength = adaptedWebGLThickness * 2.0 * arrowHeadLengthThicknessRatio;\n  float adaptedWebGLArrowHeadHalfWidth = adaptedWebGLArrowHeadLength * arrowHeadWidthLengthRatio / 2.0;\n\n  float da = a_barycentric.x;\n  float db = a_barycentric.y;\n  float dc = a_barycentric.z;\n\n  vec2 delta = vec2(\n     da * (adaptedWebGLNodeRadius * unitNormal.y)\n   + db * ((adaptedWebGLNodeRadius + adaptedWebGLArrowHeadLength) * unitNormal.y + adaptedWebGLArrowHeadHalfWidth * unitNormal.x)\n   + dc * ((adaptedWebGLNodeRadius + adaptedWebGLArrowHeadLength) * unitNormal.y - adaptedWebGLArrowHeadHalfWidth * unitNormal.x),\n\n     da * (-adaptedWebGLNodeRadius * unitNormal.x)\n   + db * (-(adaptedWebGLNodeRadius + adaptedWebGLArrowHeadLength) * unitNormal.x + adaptedWebGLArrowHeadHalfWidth * unitNormal.y)\n   + dc * (-(adaptedWebGLNodeRadius + adaptedWebGLArrowHeadLength) * unitNormal.x - adaptedWebGLArrowHeadHalfWidth * unitNormal.y)\n  );\n\n  vec2 position = (u_matrix * vec3(a_position + delta, 1)).xy;\n\n  gl_Position = vec4(position, 0, 1);\n\n  // Extract the color:\n  v_color = a_color;\n  v_color.a *= bias;\n}\n");

/***/ }),
/* 33 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("precision mediump float;\n\nvarying vec4 v_color;\n\nvoid main(void) {\n  gl_FragColor = v_color;\n}\n");

/***/ }),
/* 34 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __importDefault = (this && this.__importDefault) || function (mod) {
   return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var edge_1 = __webpack_require__(29);
var utils_1 = __webpack_require__(5);
var edge_clamped_vert_glsl_1 = __importDefault(__webpack_require__(35));
var edge_frag_glsl_1 = __importDefault(__webpack_require__(28));
var POINTS = 4, ATTRIBUTES = 6, STRIDE = POINTS * ATTRIBUTES;
var EdgeClampedProgram = /** @class */ (function (_super) {
   __extends(EdgeClampedProgram, _super);
   function EdgeClampedProgram(gl) {
      var _this = _super.call(this, gl, edge_clamped_vert_glsl_1.default, edge_frag_glsl_1.default, POINTS, ATTRIBUTES) || this;
      // Initializing indices buffer
      var indicesBuffer = gl.createBuffer();
      if (indicesBuffer === null)
         throw new Error("EdgeClampedProgram: error while getting resolutionLocation");
      _this.indicesBuffer = indicesBuffer;
      // Locations:
      _this.positionLocation = gl.getAttribLocation(_this.program, "a_position");
      _this.colorLocation = gl.getAttribLocation(_this.program, "a_color");
      _this.normalLocation = gl.getAttribLocation(_this.program, "a_normal");
      _this.radiusLocation = gl.getAttribLocation(_this.program, "a_radius");
      // Uniform locations
      var matrixLocation = gl.getUniformLocation(_this.program, "u_matrix");
      if (matrixLocation === null)
         throw new Error("EdgeClampedProgram: error while getting matrixLocation");
      _this.matrixLocation = matrixLocation;
      var sqrtZoomRatioLocation = gl.getUniformLocation(_this.program, "u_sqrtZoomRatio");
      if (sqrtZoomRatioLocation === null)
         throw new Error("EdgeClampedProgram: error while getting cameraRatioLocation");
      _this.sqrtZoomRatioLocation = sqrtZoomRatioLocation;
      var correctionRatioLocation = gl.getUniformLocation(_this.program, "u_correctionRatio");
      if (correctionRatioLocation === null)
         throw new Error("EdgeClampedProgram: error while getting viewportRatioLocation");
      _this.correctionRatioLocation = correctionRatioLocation;
      // Enabling the OES_element_index_uint extension
      // NOTE: on older GPUs, this means that really large graphs won't
      // have all their edges rendered. But it seems that the
      // `OES_element_index_uint` is quite everywhere so we'll handle
      // the potential issue if it really arises.
      // NOTE: when using webgl2, the extension is enabled by default
      _this.canUse32BitsIndices = (0, utils_1.canUse32BitsIndices)(gl);
      _this.IndicesArray = _this.canUse32BitsIndices ? Uint32Array : Uint16Array;
      _this.indicesArray = new _this.IndicesArray();
      _this.indicesType = _this.canUse32BitsIndices ? gl.UNSIGNED_INT : gl.UNSIGNED_SHORT;
      _this.bind();
      return _this;
   }
   EdgeClampedProgram.prototype.bind = function () {
      var gl = this.gl;
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);
      // Bindings
      gl.enableVertexAttribArray(this.positionLocation);
      gl.enableVertexAttribArray(this.normalLocation);
      gl.enableVertexAttribArray(this.colorLocation);
      gl.enableVertexAttribArray(this.radiusLocation);
      gl.vertexAttribPointer(this.positionLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 0);
      gl.vertexAttribPointer(this.normalLocation, 2, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 8);
      gl.vertexAttribPointer(this.colorLocation, 4, gl.UNSIGNED_BYTE, true, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 16);
      gl.vertexAttribPointer(this.radiusLocation, 1, gl.FLOAT, false, ATTRIBUTES * Float32Array.BYTES_PER_ELEMENT, 20);
   };
   EdgeClampedProgram.prototype.process = function (sourceData, targetData, data, hidden, offset) {
      if (hidden) {
         for (var i_1 = offset * STRIDE, l = i_1 + STRIDE; i_1 < l; i_1++)
            this.array[i_1] = 0;
         return;
      }
      var thickness = data.size || 1, x1 = sourceData.x, y1 = sourceData.y, x2 = targetData.x, y2 = targetData.y, radius = targetData.size || 1, color = (0, utils_1.floatColor)(data.color);
      // Computing normals
      var dx = x2 - x1, dy = y2 - y1;
      var len = dx * dx + dy * dy, n1 = 0, n2 = 0;
      if (len) {
         len = 1 / Math.sqrt(len);
         n1 = -dy * len * thickness;
         n2 = dx * len * thickness;
      }
      var i = POINTS * ATTRIBUTES * offset;
      var array = this.array;
      // First point
      array[i++] = x1;
      array[i++] = y1;
      array[i++] = n1;
      array[i++] = n2;
      array[i++] = color;
      array[i++] = 0;
      // First point flipped
      array[i++] = x1;
      array[i++] = y1;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = color;
      array[i++] = 0;
      // Second point
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = n1;
      array[i++] = n2;
      array[i++] = color;
      array[i++] = radius;
      // Second point flipped
      array[i++] = x2;
      array[i++] = y2;
      array[i++] = -n1;
      array[i++] = -n2;
      array[i++] = color;
      array[i] = -radius;
   };
   EdgeClampedProgram.prototype.computeIndices = function () {
      var l = this.array.length / ATTRIBUTES;
      var size = l + l / 2;
      var indices = new this.IndicesArray(size);
      for (var i = 0, c = 0; i < l; i += 4) {
         indices[c++] = i;
         indices[c++] = i + 1;
         indices[c++] = i + 2;
         indices[c++] = i + 2;
         indices[c++] = i + 1;
         indices[c++] = i + 3;
      }
      this.indicesArray = indices;
   };
   EdgeClampedProgram.prototype.bufferData = function () {
      _super.prototype.bufferData.call(this);
      // Indices data
      var gl = this.gl;
      gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.indicesArray, gl.STATIC_DRAW);
   };
   EdgeClampedProgram.prototype.render = function (params) {
      if (this.hasNothingToRender())
         return;
      var gl = this.gl;
      var program = this.program;
      gl.useProgram(program);
      // Binding uniforms
      gl.uniformMatrix3fv(this.matrixLocation, false, params.matrix);
      gl.uniform1f(this.sqrtZoomRatioLocation, Math.sqrt(params.ratio));
      gl.uniform1f(this.correctionRatioLocation, params.correctionRatio);
      // Drawing:
      gl.drawElements(gl.TRIANGLES, this.indicesArray.length, this.indicesType, 0);
   };
   return EdgeClampedProgram;
}(edge_1.AbstractEdgeProgram));
exports["default"] = EdgeClampedProgram;


/***/ }),
/* 35 */
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

"use strict";
__webpack_require__.r(__webpack_exports__);
/* harmony export */ __webpack_require__.d(__webpack_exports__, {
/* harmony export */   "default": () => (__WEBPACK_DEFAULT_EXPORT__)
/* harmony export */ });
/* harmony default export */ const __WEBPACK_DEFAULT_EXPORT__ = ("attribute vec4 a_color;\nattribute vec2 a_normal;\nattribute vec2 a_position;\nattribute float a_radius;\n\nuniform mat3 u_matrix;\nuniform float u_sqrtZoomRatio;\nuniform float u_correctionRatio;\n\nvarying vec4 v_color;\nvarying vec2 v_normal;\nvarying float v_thickness;\n\nconst float minThickness = 1.7;\nconst float bias = 255.0 / 254.0;\nconst float arrowHeadLengthThicknessRatio = 2.5;\n\nvoid main() {\n  float normalLength = length(a_normal);\n  vec2 unitNormal = a_normal / normalLength;\n\n  // These first computations are taken from edge.vert.glsl. Please read it to\n  // get better comments on what's happening:\n  float pixelsThickness = max(normalLength, minThickness * u_sqrtZoomRatio);\n  float webGLThickness = pixelsThickness * u_correctionRatio;\n  float adaptedWebGLThickness = webGLThickness * u_sqrtZoomRatio;\n\n  // Here, we move the point to leave space for the arrow head:\n  float direction = sign(a_radius);\n  float adaptedWebGLNodeRadius = direction * a_radius * 2.0 * u_correctionRatio * u_sqrtZoomRatio;\n  float adaptedWebGLArrowHeadLength = adaptedWebGLThickness * 2.0 * arrowHeadLengthThicknessRatio;\n\n  vec2 compensationVector = vec2(-direction * unitNormal.y, direction * unitNormal.x) * (adaptedWebGLNodeRadius + adaptedWebGLArrowHeadLength);\n\n  // Here is the proper position of the vertex\n  gl_Position = vec4((u_matrix * vec3(a_position + unitNormal * adaptedWebGLThickness + compensationVector, 1)).xy, 0, 1);\n\n  v_thickness = webGLThickness / u_sqrtZoomRatio;\n\n  v_normal = unitNormal;\n  v_color = a_color;\n  v_color.a *= bias;\n}\n");

/***/ }),
/* 36 */
/***/ (function(__unused_webpack_module, exports, __webpack_require__) {

"use strict";

var __extends = (this && this.__extends) || (function () {
   var extendStatics = function (d, b) {
      extendStatics = Object.setPrototypeOf ||
         ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
         function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
      return extendStatics(d, b);
   };
   return function (d, b) {
      if (typeof b !== "function" && b !== null)
         throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
   };
})();
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
   if (k2 === undefined) k2 = k;
   Object.defineProperty(o, k2, { enumerable: true, get: function() { return m[k]; } });
}) : (function(o, m, k, k2) {
   if (k2 === undefined) k2 = k;
   o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
   Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
   o["default"] = v;
});
var __importStar = (this && this.__importStar) || function (mod) {
   if (mod && mod.__esModule) return mod;
   var result = {};
   if (mod != null) for (var k in mod) if (k !== "default" && Object.prototype.hasOwnProperty.call(mod, k)) __createBinding(result, mod, k);
   __setModuleDefault(result, mod);
   return result;
};
var __read = (this && this.__read) || function (o, n) {
   var m = typeof Symbol === "function" && o[Symbol.iterator];
   if (!m) return o;
   var i = m.call(o), r, ar = [], e;
   try {
      while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
   }
   catch (error) { e = { error: error }; }
   finally {
      try {
         if (r && !r.done && (m = i["return"])) m.call(i);
      }
      finally { if (e) throw e.error; }
   }
   return ar;
};
Object.defineProperty(exports, "__esModule", ({ value: true }));
var captor_1 = __importStar(__webpack_require__(13));
var DRAG_TIMEOUT = 200;
var TOUCH_INERTIA_RATIO = 3;
var TOUCH_INERTIA_DURATION = 200;
/**
 * Touch captor class.
 *
 * @constructor
 */
var TouchCaptor = /** @class */ (function (_super) {
   __extends(TouchCaptor, _super);
   function TouchCaptor(container, renderer) {
      var _this = _super.call(this, container, renderer) || this;
      _this.enabled = true;
      _this.isMoving = false;
      _this.hasMoved = false;
      _this.touchMode = 0; // number of touches down
      _this.startTouchesPositions = [];
      // Binding methods:
      _this.handleStart = _this.handleStart.bind(_this);
      _this.handleLeave = _this.handleLeave.bind(_this);
      _this.handleMove = _this.handleMove.bind(_this);
      // Binding events
      container.addEventListener("touchstart", _this.handleStart, false);
      container.addEventListener("touchend", _this.handleLeave, false);
      container.addEventListener("touchcancel", _this.handleLeave, false);
      container.addEventListener("touchmove", _this.handleMove, false);
      return _this;
   }
   TouchCaptor.prototype.kill = function () {
      var container = this.container;
      container.removeEventListener("touchstart", this.handleStart);
      container.removeEventListener("touchend", this.handleLeave);
      container.removeEventListener("touchcancel", this.handleLeave);
      container.removeEventListener("touchmove", this.handleMove);
   };
   TouchCaptor.prototype.getDimensions = function () {
      return {
         width: this.container.offsetWidth,
         height: this.container.offsetHeight,
      };
   };
   TouchCaptor.prototype.dispatchRelatedMouseEvent = function (type, e, touch, emitter) {
      var mousePosition = touch || e.touches[0];
      var mouseEvent = new MouseEvent(type, {
         clientX: mousePosition.clientX,
         clientY: mousePosition.clientY,
         altKey: e.altKey,
         ctrlKey: e.ctrlKey,
      });
      mouseEvent.isFakeSigmaMouseEvent = true;
      (emitter || this.container).dispatchEvent(mouseEvent);
   };
   TouchCaptor.prototype.handleStart = function (e) {
      var _this = this;
      if (!this.enabled)
         return;
      // Prevent default to avoid default browser behaviors...
      e.preventDefault();
      // ...but simulate mouse behavior anyway, to get the MouseCaptor working as well:
      if (e.touches.length === 1)
         this.dispatchRelatedMouseEvent("mousedown", e);
      var touches = (0, captor_1.getTouchesArray)(e.touches);
      this.touchMode = touches.length;
      this.startCameraState = this.renderer.getCamera().getState();
      this.startTouchesPositions = touches.map(function (touch) { return (0, captor_1.getPosition)(touch, _this.container); });
      this.lastTouches = touches;
      this.lastTouchesPositions = this.startTouchesPositions;
      // When there are two touches down, let's record distance and angle as well:
      if (this.touchMode === 2) {
         var _a = __read(this.startTouchesPositions, 2), _b = _a[0], x0 = _b.x, y0 = _b.y, _c = _a[1], x1 = _c.x, y1 = _c.y;
         this.startTouchesAngle = Math.atan2(y1 - y0, x1 - x0);
         this.startTouchesDistance = Math.sqrt(Math.pow(x1 - x0, 2) + Math.pow(y1 - y0, 2));
      }
      this.emit("touchdown", (0, captor_1.getTouchCoords)(e, this.container));
   };
   TouchCaptor.prototype.handleLeave = function (e) {
      if (!this.enabled)
         return;
      // Prevent default to avoid default browser behaviors...
      e.preventDefault();
      // ...but simulate mouse behavior anyway, to get the MouseCaptor working as well:
      if (e.touches.length === 0 && this.lastTouches && this.lastTouches.length) {
         this.dispatchRelatedMouseEvent("mouseup", e, this.lastTouches[0], document);
         // ... and only click if no move was made
         if (!this.hasMoved) {
            this.dispatchRelatedMouseEvent("click", e, this.lastTouches[0]);
         }
      }
      if (this.movingTimeout) {
         this.isMoving = false;
         clearTimeout(this.movingTimeout);
      }
      switch (this.touchMode) {
         case 2:
            if (e.touches.length === 1) {
               this.handleStart(e);
               e.preventDefault();
               break;
            }
         /* falls through */
         case 1:
            // TODO
            // Dispatch event
            if (this.isMoving) {
               var camera = this.renderer.getCamera();
               var cameraState = camera.getState(), previousCameraState = camera.getPreviousState() || { x: 0, y: 0 };
               camera.animate({
                  x: cameraState.x + TOUCH_INERTIA_RATIO * (cameraState.x - previousCameraState.x),
                  y: cameraState.y + TOUCH_INERTIA_RATIO * (cameraState.y - previousCameraState.y),
               }, {
                  duration: TOUCH_INERTIA_DURATION,
                  easing: "quadraticOut",
               });
            }
            this.hasMoved = false;
            this.isMoving = false;
            this.touchMode = 0;
            break;
      }
      this.emit("touchup", (0, captor_1.getTouchCoords)(e, this.container));
   };
   TouchCaptor.prototype.handleMove = function (e) {
      var _a;
      var _this = this;
      if (!this.enabled)
         return;
      // Prevent default to avoid default browser behaviors...
      e.preventDefault();
      // ...but simulate mouse behavior anyway, to get the MouseCaptor working as well:
      if (e.touches.length === 1)
         this.dispatchRelatedMouseEvent("mousemove", e);
      var touches = (0, captor_1.getTouchesArray)(e.touches);
      var touchesPositions = touches.map(function (touch) { return (0, captor_1.getPosition)(touch, _this.container); });
      this.lastTouches = touches;
      this.lastTouchesPositions = touchesPositions;
      // If a move was initiated at some point and we get back to startpoint,
      // we should still consider that we did move (which also happens after a
      // multiple touch when only one touch remains in which case handleStart
      // is recalled within handleLeave).
      // Now, some mobile browsers report zero-distance moves so we also check that
      // one of the touches did actually move from the origin position.
      this.hasMoved || (this.hasMoved = touchesPositions.some(function (position, idx) {
         var startPosition = _this.startTouchesPositions[idx];
         return position.x !== startPosition.x || position.y !== startPosition.y;
      }));
      // If there was no move, do not trigger touch moves behavior
      if (!this.hasMoved) {
         return;
      }
      this.isMoving = true;
      if (this.movingTimeout)
         clearTimeout(this.movingTimeout);
      this.movingTimeout = window.setTimeout(function () {
         _this.isMoving = false;
      }, DRAG_TIMEOUT);
      var camera = this.renderer.getCamera();
      var startCameraState = this.startCameraState;
      switch (this.touchMode) {
         case 1: {
            var _b = this.renderer.viewportToFramedGraph((this.startTouchesPositions || [])[0]), xStart = _b.x, yStart = _b.y;
            var _c = this.renderer.viewportToFramedGraph(touchesPositions[0]), x = _c.x, y = _c.y;
            camera.setState({
               x: startCameraState.x + xStart - x,
               y: startCameraState.y + yStart - y,
            });
            break;
         }
         case 2: {
            /**
             * Here is the thinking here:
             *
             * 1. We can find the new angle and ratio, by comparing the vector from "touch one" to "touch two" at the start
             *   of the d'n'd and now
             *
             * 2. We can use `Camera#viewportToGraph` inside formula to retrieve the new camera position, using the graph
             *   position of a touch at the beginning of the d'n'd (using `startCamera.viewportToGraph`) and the viewport
             *   position of this same touch now
             */
            var newCameraState = {};
            var _d = touchesPositions[0], x0 = _d.x, y0 = _d.y;
            var _e = touchesPositions[1], x1 = _e.x, y1 = _e.y;
            var angleDiff = Math.atan2(y1 - y0, x1 - x0) - this.startTouchesAngle;
            var ratioDiff = Math.hypot(y1 - y0, x1 - x0) / this.startTouchesDistance;
            // 1.
            var newRatio = camera.getBoundedRatio(startCameraState.ratio / ratioDiff);
            newCameraState.ratio = newRatio;
            newCameraState.angle = startCameraState.angle + angleDiff;
            // 2.
            var dimensions = this.getDimensions();
            var touchGraphPosition = this.renderer.viewportToFramedGraph((this.startTouchesPositions || [])[0], { cameraState: startCameraState });
            var smallestDimension = Math.min(dimensions.width, dimensions.height);
            var dx = smallestDimension / dimensions.width;
            var dy = smallestDimension / dimensions.height;
            var ratio = newRatio / smallestDimension;
            // Align with center of the graph:
            var x = x0 - smallestDimension / 2 / dx;
            var y = y0 - smallestDimension / 2 / dy;
            // Rotate:
            _a = __read([
               x * Math.cos(-newCameraState.angle) - y * Math.sin(-newCameraState.angle),
               y * Math.cos(-newCameraState.angle) + x * Math.sin(-newCameraState.angle),
            ], 2), x = _a[0], y = _a[1];
            newCameraState.x = touchGraphPosition.x - x * ratio;
            newCameraState.y = touchGraphPosition.y + y * ratio;
            camera.setState(newCameraState);
            break;
         }
      }
      this.emit("touchmove", (0, captor_1.getTouchCoords)(e, this.container));
   };
   return TouchCaptor;
}(captor_1.default));
exports["default"] = TouchCaptor;


/***/ }),
/* 37 */
/***/ ((__unused_webpack_module, exports) => {

"use strict";

Object.defineProperty(exports, "__esModule", ({ value: true }));
exports.doEdgeCollideWithPoint = exports.isPixelColored = void 0;
/**
 * This helper returns true is the pixel at (x,y) in the given WebGL context is
 * colored, and false else.
 */
function isPixelColored(gl, x, y) {
   var pixels = new Uint8Array(4);
   gl.readPixels(x, gl.drawingBufferHeight - y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
   return pixels[3] > 0;
}
exports.isPixelColored = isPixelColored;
/**
 * This helper checks whether or not a point (x, y) collides with an
 * edge, connecting a source (xS, yS) to a target (xT, yT) with a thickness in
 * pixels.
 */
function doEdgeCollideWithPoint(x, y, xS, yS, xT, yT, thickness) {
   // Check first if point is out of the rectangle which opposite corners are the
   // source and the target, rectangle we expand by `thickness` in every
   // directions:
   if (x < xS - thickness && x < xT - thickness)
      return false;
   if (y < yS - thickness && y < yT - thickness)
      return false;
   if (x > xS + thickness && x > xT + thickness)
      return false;
   if (y > yS + thickness && y > yT + thickness)
      return false;
   // Check actual collision now: Since we now the point is in this big rectangle
   // we "just" need to check that the distance between the point and the line
   // connecting the source and the target is less than `thickness`:
   // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
   var distance = Math.abs((xT - xS) * (yS - y) - (xS - x) * (yT - yS)) / Math.sqrt(Math.pow(xT - xS, 2) + Math.pow(yT - yS, 2));
   return distance < thickness / 2;
}
exports.doEdgeCollideWithPoint = doEdgeCollideWithPoint;


/***/ })
/******/    ]);
/************************************************************************/
/******/    // The module cache
/******/    var __webpack_module_cache__ = {};
/******/
/******/    // The require function
/******/    function __webpack_require__(moduleId) {
/******/       // Check if module is in cache
/******/       var cachedModule = __webpack_module_cache__[moduleId];
/******/       if (cachedModule !== undefined) {
/******/          return cachedModule.exports;
/******/       }
/******/       // Create a new module (and put it into the cache)
/******/       var module = __webpack_module_cache__[moduleId] = {
/******/          // no module.id needed
/******/          // no module.loaded needed
/******/          exports: {}
/******/       };
/******/
/******/       // Execute the module function
/******/       __webpack_modules__[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/       // Return the exports of the module
/******/       return module.exports;
/******/    }
/******/
/************************************************************************/
/******/    /* webpack/runtime/define property getters */
/******/    (() => {
/******/       // define getter functions for harmony exports
/******/       __webpack_require__.d = (exports, definition) => {
/******/          for(var key in definition) {
/******/             if(__webpack_require__.o(definition, key) && !__webpack_require__.o(exports, key)) {
/******/                Object.defineProperty(exports, key, { enumerable: true, get: definition[key] });
/******/             }
/******/          }
/******/       };
/******/    })();
/******/
/******/    /* webpack/runtime/hasOwnProperty shorthand */
/******/    (() => {
/******/       __webpack_require__.o = (obj, prop) => (Object.prototype.hasOwnProperty.call(obj, prop))
/******/    })();
/******/
/******/    /* webpack/runtime/make namespace object */
/******/    (() => {
/******/       // define __esModule on exports
/******/       __webpack_require__.r = (exports) => {
/******/          if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/             Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/          }
/******/          Object.defineProperty(exports, '__esModule', { value: true });
/******/       };
/******/    })();
/******/
/************************************************************************/
/******/
/******/    // startup
/******/    // Load entry module and return exports
/******/    // This entry module is referenced by other modules so it can't be inlined
/******/    var __webpack_exports__ = __webpack_require__(0);
/******/    Sigma = __webpack_exports__;
/******/
/******/ })()
;
