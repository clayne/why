#!/usr/bin/env node
let _ = require("lodash");

const chalk = require("chalk");
const {lt} = require("dominators");

const Node = require("./node.js");
const {alpha, numerize} = require("./util.js");

/**
 * Contains various utilities.
 * 
 * @module util
 */

const getID = Node.getID;

/**
 * Represents a directed graph datatype.
 */
class Graph {
	/**
	 * Creates a new graph.
	 * @param {number} n The number of nodes in the graph.
	 */
	constructor(n) {
		/**
		 * An array of all the nodes in the graph.
		 * @type {Array<Node>}
		 * @name module:util~Graph#nodes
		 */

		return new Proxy(this.reset(n), {
			get(target, prop) {
				if (typeof prop == "symbol") {
					return target[prop];
				}

				if (Number(prop) == prop) {
					return target.getNode(prop);
				}

				if (prop in Array.prototype && !(prop in target)) {
					if (typeof Array.prototype[prop] == "function") {
						return target.nodes[prop].bind(target.nodes);
					}
					
					return target.nodes[prop];
				}

				return target[prop];
			},

			set(target, prop, val) {
				if (Number(prop) != prop) {
					return Reflect.set(...arguments);
				}
				
				const i = _.findIndex(target.nodes, node => node.id == prop);
				if (i == -1) {
					return Reflect.set(...arguments);
				}

				target.nodes[i] = val;
				return true;
			}
		});
	}

	/**
	 * Deletes all nodes in the graph.
	 * @param {number|Array} n The number of new empty nodes to replace the old nodes, or an array of IDs to make new nodes with.
	 */
	reset(n) {
		if (n == undefined) {
			this.nodes = [];
		} else if (typeof n == "number") {
			this.nodes = _.range(0, n).map(i => new Node(i, this));
		} else {
			this.nodes = _.range(0, n.length).map(i => new Node(n[i], this));
		}

		return this;
	}

	add(data) {
		const newNode = new Node(this.length, this, data);
		this.push(newNode);
		return newNode;
	}

	/**
	 * Returns the node with a given ID.
	 * @param  {Node|string|number} n The ID of the node to return.
	 * @return {Node} The node corresponding to n if n is a number; n otherwise.
	 */
	getNode(n) {
		if (n instanceof Node) {
			return n;
		}

		if (n == undefined) {
			throw new Error("Graph.getID() called with undefined");
		}

		n = numerize(n);
		return _.find(this.nodes, node => numerize(node.id) == n);
	}

	/**
	 * Adds a unidirectional connection from one node to another.
	 * @param {(Node|number)} source      The source node.
	 * @param {(Node|number)} destination The destination node.
	 */
	arc(source, destination) {
		this.getNode(source).arc(destination);
		return this;
	}

	arcString(str) {
		str.split(/\s+/).forEach(s => this.arc(...s.split("").map(c => alpha.indexOf(c.toLowerCase()))));
		return this;
	}

	/**
	 * Batch-adds arcs from an array of [source, ...destinations] sets.
	 * @param {...Array<Array<number, ...number>>} arcs - An array of arc sets to add.
	 */
	arcs(...sets) {
		sets.forEach(([src, ...dests]) => dests.forEach(dest => this.arc(src, dest)));
		return this;
	}

	/**
	 * Removes an edge from one node to another.
	 * @param {(Node|number)} source      The source node.
	 * @param {(Node|number)} destination The destination node.
	 */
	removeArc(source, destination) {
		this.nodes[getID(source)].removeArc(destination);
		return this;
	}

	/**
	 * Adds a bidirectional connection between two nodes.
	 * @param {(Node|number)} a The first node.
	 * @param {(Node|number)} b The second node.
	 */
	edge(a, b) {
		this.nodes[getID(a)].arc(b);
		this.nodes[getID(b)].arc(a);
		return this;
	}

	/**
	 * Removes all connections between two nodes.
	 * @param {(Node|number)} a The first node.
	 * @param {(Node|number)} b The second node.
	 */
	disconnect(a, b) {
		this.nodes[getID(a)].removeArc(b);
		this.nodes[getID(b)].removeArc(a);
		return this;
	}

	findSingle(predicate) {
		const found = this.nodes.filter(predicate);
		if (found.length != 1) {
			throw `Predicate matched ${found.length} results`;
		}

		return found[0];
	}

	/**
	 * Returns an array whose length is equal to the number of nodes in this graph
	 * and fills it with a predefined value.
	 * @param  {*} [value=null] A value to fill the array with.
	 * @return {Array} The filled array.
	 */
	fill(value=null) {
		return _.fill(Array(this.length), value);
	}

	/**
	 * Returns an object whose keys are the IDs of each node in the graph
	 * and whose values are all equal to the given value.
	 * @param  {*} value A value to use as the value of each entry in the object.
	 * @return {Object} The filled object.
	 */
	fillObj(value=null) {
		const out = {};
		this.forEach(v => out[v.id] = value);
		return out;
	}

	/**
	 * Returns an object whose keys are the IDs of each node in the graph and whose values
	 * are the return value of the given function when called with a Node object.
	 * @param  {Function} fn A function that takes a Node and returns any value.
	 * @return {Object} The mapped object.
	 */
	mapValues(fn) {
		const out = {};
		this.forEach((v, i) => out[v.id] = fn(v, i));
		return out;
	}

	/**
	 * Renames all nodes in the graph such that they have numeric IDs in the range [offset, n + offset).
	 * @param  {number} [offset=0] The starting point of the range.
	 * @return {Object<string|number, number>} A map of old IDs to new IDs.
	 */
	normalize(offset=0) {
		const renameMap = this.mapValues((v, i) => i + offset);
		const oldNodes = Object.values(this.nodes);

		this.nodes = [];
		oldNodes.forEach(v => {
			["in", "out"].forEach(p => v[p] = v[p].map(i => renameMap[i]))
			v.id = renameMap[v.id];
			this.nodes.push(v);
		});

		return renameMap;
	}

	dTree(startID = 0) {
		const lentar = this.lengauerTarjan(startID);
		const out = new Graph(Object.keys(lentar).map(numerize));
		Object.entries(lentar).forEach(([k, v]) => out.arc(v == undefined? k : v, k));
		return out;
	}

	static strictDominators(dt) {
		const out = {};
		for (const node of dt.nodes) {
			let parent = dt[node.in[0]];
			out[node.id] = [];
			while (parent) {
				out[node.id].push(parent.id);
				if (parent.in[0] == parent.id) {
					break;
				}

				parent = dt[parent.in[0]];
			}
		}

		return out;
	}

	allEdges() {
		return this.reduce((a, {id: src, out}) =>
			[...a, ...out.map(dst => [src, dst])],
		[]);
	}

	djTree(startID=0, bidirectional=false) {
		// TODO: should all D-edges be bidirectional?
		const dt = this.dTree(startID), sdom = Graph.strictDominators(dt);
		this.allEdges()
			.filter(([src, dst]) => !sdom[src].includes(dst))
			.forEach(bidirectional?
				p => dt.edge(...p)
			  : p => dt.arc(...p));
		return dt;
	}

	/**
	 * Finds the dominators of each node given a start node using the Lengauer-Tarjan algorithm.
	 * This is a wrapper for the `lt` function from the `dominators` package by Julian Jensen.
	 * @param  {number|string} [startID=0] The ID of the start node.
	 * @return {Object<number|string, number|string>} A map of node IDs to the IDs of their dominators.
	 */
	lengauerTarjan(startID=0) {
		const normalized = this.clone();
		const renames = _.mapValues(_.invert(normalized.normalize()), v => numerize(v));
		const formatted = normalized.map(v => v.out);
		return lt(formatted, startID).reduce((a, b, i) => ({...a, [renames[i]]: renames[b]}), {});
	}

	/**
	 * @typedef {Object} DFSResult
	 * @property {number[]} parents    A list of each node's parent (null if nonexistent).
	 * @property {number[]} discovered A list of the times each node was discovered.
	 * @property {number[]} finished   A list of the times each node was finished.
	 */

	/**
	 * Runs a depth-first search on the graph.
	 * @param  {number|string} [startID=0] The ID of the start node.
	 * @return {module:util~DFSResult} The result of the search.
	 */
	dfs(startID=0) {
		const n = this.nodes.length;
		const parents    = this.fill(null);
		const discovered = this.fill(null);
		const finished   = this.fill(null);
		let time = 0;

		const visit = u => {
			discovered[u] = ++time;
			this.nodes[u].out.sort().forEach(v => {
				if (discovered[v] == null) {
					parents[v] = u;
					visit(v);
				}
			});

			finished[u] = ++time;
		};

		visit(startID);
		return {parents, discovered, finished};
	}

	/**
	 * Renames the node with a given ID (if one exists) to a new ID.
	 * @param  {number|string} oldID The old ID of the node to rename.
	 * @param  {number|string} newID The new ID to assign to the node.
	 * @return {Graph} The same graph the method was called on.
	 */
	renameNode(oldID, newID) {
		const node = this.getNode(oldID);
		if (node) {
			node.rename(newID);
		}

		return this;
	}

	/**
	 * Returns an array of this graph's connected components using Kosaraju's algorithm.
	 * @return {Array<Array<Node>>} An array of connected components.
	 */
	components() {
		const visited = this.fill(false);
		const parents = this.fill(null);
		const components = {}; 
		const l = [];

		const visit = u => {
			if (!visited[u]) {
				visited[u] = true;
				this.nodes[u].out.forEach(visit);
				l.unshift(u);
			}
		};

		const assign = (u, root) => {
			if (parents[u] == null) {
				parents[u] = root;
				if (!components[root]) {
					components[root] = [u];
				} else {
					components[root].push(u);
				}

				this.getNode(u).in.forEach(v => assign(v, root));
			}
		};

		this.nodes.forEach((node, u) => visit(u));
		l.forEach(u => assign(u, u));

		return Object.values(components).map(a => a.map(u => this.nodes[u]));
	}

	/**
	 * Runs a DFS on each node that hasn't been visited, appending each discovered node to the output array,
	 * until no unvisited nodes remain. The initial list of unvisited nodes is ordered the same as the `nodes`
	 * array of this Graph object.
	 * @return {Node[]} An array of ordered nodes.
	 */
	sortedDFS() {
		const list = [];
		const visited = this.fill(false);
		const unvisited = _.range(0, this.length);

		const visit = u => {
			visited[u] = true;
			_.pull(unvisited, u);

			for (const v of this.nodes[u].out.sort()) {
				if (!visited[v]) {
					visit(v);
				}
			}
			
			list.push(u);
		};
		
		while (unvisited.length) {
			visit(unvisited[0]);
		}

		return list.map(n => this.nodes[n]);
	}

	/**
	 * Calculates a topologically sorted list of nodes using Kahn's algorithm.
	 * @return {Node[]} A topologically sorted list of the graph's nodes.
	 * @throws Will throw an error if the graph is cyclic.
	 */
	topoSort() {
		let copy = this.clone();
		const l = [], s = copy.nodes.filter(node => !node.in.length);
		if (1 < this.nodes.length && !s.length) {
			// If there are multiple nodes in the graph and none of them lack in-edges, the graph has to be cyclic.
			// The converse isn't necessarily true, so this is just an preliminary check.
			throw new Error("Graph is cyclic.");
		}

		while (s.length) {
			let n = s.pop();
			l.unshift(n);
			
			copy.nodes.filter(m => m != n && m.connectsFrom(n)).forEach(m => {
				m.removeArcFrom(n);
				
				if (!m.in.length) {
					s.unshift(m);
				}
			});
		}

		copy.nodes.forEach(node => {
			if (node.out.length) {
				throw new Error("Graph contains a cycle.");
			}
		});

		return l.map(node => this.nodes[node.id]);
	}

	/**
	 * Removes all loop edges from the graph.
	 */
	removeLoops() {
		this.nodes.forEach(node => this.disconnect(node, node));
		return this;
	}

	/**
	 * Condenses a list of nodes into a single node, removes the old nodes from the graph and inserts the new node.
	 * The new node's in/out arrays are the unions of the given nodes' in/out arrays.
	 * The new node is reflexive if any of the given nodes is reflexive.
	 * @param  {Array<Node | number>} nodes A list of nodes.
	 * @return {Node} The coalesced node.
	 */
	coalesce(nodes) {
		if (!nodes.length) {
			return undefined;
		}

		// The input may contain either Nodes or numeric IDs. Convert all the IDs to nodes.
		nodes = nodes.map(n => this.nodes[getID(n)]);

		// Calculate the union of all in/out edges, but don't include edges between any of the given nodes.
		const combinedIn  = _.without(_.union(...nodes.map(node => node.in)),  ...nodes);
		const combinedOut = _.without(_.union(...nodes.map(node => node.out)), ...nodes);

		const reflexive = _.some(nodes, n => n.isReflexive);

		const allIDs = nodes.map(node => node.id);
		const newID = nodes[0].id;
		const oldIDs = _.without(allIDs, newID);
		let newNode = this.getNode(newID);
		
		// Remove all the old nodes from the graph.
		_.remove(this.nodes, v => allIDs.includes(getID(v)));

		// Go through every remaining node's in/out arrays, remove the old node IDs
		// and insert the new ID where applicable.
		for (const node of this.nodes) {
			if (_.intersection(node.in,   allIDs).length)
				node.in  = _.sortBy(_.without(node.in,  ...allIDs).concat(newID), getID);
			if (_.intersection(node.out,  allIDs).length)
				node.out = _.sortBy(_.without(node.out, ...allIDs).concat(newID), getID);
		}

		if (!newNode) {
			newNode = new Node(newID, this);
		}

		newNode.in  = _.without(combinedIn,  ...allIDs);
		newNode.out = _.without(combinedOut, ...allIDs);
		
		if (reflexive) {
			newNode.in.push(newID);
			newNode.out.push(newID);
		}

		newNode.in  = _.sortBy(newNode.in,  getID);
		newNode.out = _.sortBy(newNode.out, getID);
		
		this.nodes.push(newNode);
		return newNode;
	}

	/**
	 * Calculates and returns the transpose of the graph.
	 * @type {Graph}
	 */
	get transpose() {
		let graph = new Graph(this.nodes.length);
		this.nodes.forEach(({out}, u) => out.forEach(v => graph.arc(v, u)));

		return graph;
	}

	/**
	 * Returns a copy of this graph.
	 * @return {Graph} A copy of the graph.
	 */
	clone() {
		let newGraph = new Graph(this.nodes.length);
		newGraph.nodes = this.nodes.map(node => node.clone(newGraph));
		return newGraph;
	}

	/**
	 * Returns a string containing each node's adjacency list.
	 * @return {string} A string representation of the graph.
	 */
	toString(idFn = x=>x, outFn) {
		if (outFn === undefined) {
			outFn = idFn;
		}
		
		return _.sortBy(this.nodes, "id").map((node) => `${idFn(node.id, node)} => ${node.out.map(out => outFn(out, node)).join(", ")}`).join("\n");
	}

	validateDirections() {
		for (const node of this.nodes) {
			for (const o of node.out) {
				if (!this[o].in.includes(node.id)) {
					return false;
				}
			}

			for (const i of node.in) {
				if (!this[i].out.includes(node.id)) {
					return false;
				}
			}
		}

		return true;
	}
}

module.exports = Graph;
module.exports.Node = Node;
