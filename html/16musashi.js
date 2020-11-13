const grid_size = 50;
const half_grid_size = grid_size / 2;
function grid_x(x) {
  return x * grid_size + half_grid_size;
};
function grid_y(y) {
  return y * grid_size + half_grid_size;
};

const dp = [[1, 0], [1, 1], [0, 1], [-1, 1], [-1, 0], [-1, -1], [0, -1], [1, -1]];
const has_brown = [[1, 1, 1, 1, 1, 0, 0], [1, 0, 0, 0, 1, 0, 0], [1, 0, 0, 0, 1, 0, 0], [1, 0, 0, 0, 1, 0, 0], [1, 1, 1, 1, 1, 0, 0]];
const offset_table = 
      [[[14, 15, 16, 17, 18, -1, 19],
	[ 7,  8,  9, 10, 11, 12, 13],
	[ 0,  1,  2,  3,  4,  5,  6],
	[20, 21, 22, 23, 24, 25, 26],
	[27, 28, 29, 30, 31, -1, 32]],
       [[13, 14, 15, 16, 17, -1, 18],
	[ 7,  8,  9, 10, 11, 12, -1],
	[ 0,  1,  2,  3,  4,  5,  6],
	[19, 20, 21, 22, 23, 24, -1],
	[25, 26, 27, 28, 29, -1, 30]],
       [[10, 11, 12, 13, 14, -1, -1],
	[ 5,  6,  7,  8,  9, -1, -1],
	[ 0,  1,  2,  3,  4, -1, -1],
	[15, 16, 17, 18, 19, -1, -1],
	[20, 21, 22, 23, 24, -1, -1]],
      ];
var app = new Vue({
  el: "#appline",
  data: {
    colors : ['black', 'brown'],
    rs : [10, 5],
    points: [],
    lines: [],
    pieces: [], 
    moving: null,
    init: null, 
    markers: [],
    message: "",
    mode : 0,
    turn : 1,
    bval : 0,
    history : [], 
    cur : 0,
    size : 0,
    psize : [33, 31, 25],
  },
  computed: {
    bf: function() {
      this.board_value(this);
      return this.bval;
    }
  },   
  methods: {
    has_point: function (x, y) {
      if (x < 0 || 7 <= x) { return false;}
      if (y < 0 || 5 <= y) { return false;}
      return offset_table[this.size][y][x] >= 0;
    },
    has_line: function (x1, y1, x2, y2) {
      if (!this.has_point(x1, y1) || !this.has_point(x2, y2)) {
	return false;
      }
      if (this.size == 1 && y1 == y2 && x1 + x2 == 5 + 6) {
	return false;
      }
      if (x1 == x2 || y1 == y2) {
	if (y1 == y2 && y1 != 2 && Math.max(x1, x2) == 5) {
	  return false;
	} 
	return true;
      }  
      if ((x2 == 5 && x1 == 4 && (y1 == 0 || y1 == 4)) ||
	  (x1 == 5 && x2 == 4 && (y2 == 0 || y2 == 4))) {
	return false;
      }
      if (this.size == 1 && ((x1 == y1 && x2 == y2) || (x1 == 4 - y1 && x2 == 4 - y2))) {
	return false;
      }
      return (x1 + y1) % 2 == 0;
    },
    board_value_flip: function(state, flip=false) {
      let black = state.pieces.filter(p1 => p1.turn === 0)[0];
      // console.log('state.pieces=' + JSON.stringify(state.pieces) + ',black=' + JSON.stringify(black));
      let black_bit = offset_table[this.size][(flip ? 4 - black.y : black.y)][black.x];
      let v = black_bit * Math.pow(2, this.psize[this.size]);
      for (let brown of state.pieces.filter(p1 => p1.turn === 1)) {
	let brown_bit = offset_table[this.size][(flip ? 4 - brown.y : brown.y)][brown.x];
	v += Math.pow(2, brown_bit);
      }
      v += state.turn * Math.pow(2, black_bit);
      return v;
    },
    board_value: async function(state) {
      let v = Math.min(this.board_value_flip(state), this.board_value_flip(state, true));
      let range = 'bytes=' + v + '-' + v; 
      const response = await fetch('https://gps.tanaka.ecc.u-tokyo.ac.jp/16musashi/count_' + this.psize[this.size] + '_0.bin',
				   {headers :
				    {"Content-Type" : "application/octet-stream",
				     "range" : range}});
      const buffer = await response.arrayBuffer();
      const ibuffer = await new Uint8Array(buffer);
      state.bval = await ibuffer[0];
    },
    remove_piece: function(p) {
      this.pieces = this.pieces.filter(p1 => p1.x !== p.x || p1.y !== p.y);
    },
    neighbors: function(p, is_half=false) {
      let r = []
      let [x, y] = [p.x, p.y];
      for (let d = 0; d < (is_half ? 4 : 8); d++) {
	let [dx, dy] = dp[d];
	if (this.size == 1 && x == 6 && dx == 0) {
	  dy *= 2;
	}
	if (this.has_line(x, y, x + dx, y + dy)) {
	  r.push({x : x + dx, y : y + dy});
	}
      }
      return r;
    },
    is_piece: function(p) {
      return this.pieces.some(p1 => p1.x == p.x && p1.y == p.y);
    },
    capture_browns: function(p, pieces) {
      let ns = this.neighbors(p);
      for (let i = 1; i < ns.length; i++) {
	if (!pieces.some(p1 => p1.x == ns[i].x && p1.y == ns[i].y)) {
	  continue;
	}
	for (let j = 0; j < i; j++) {
	  if (!pieces.some(p1 => p1.x == ns[j].x && p1.y == ns[j].y))
	    continue;
	  if (ns[i].x + ns[j].x !== p.x * 2 || ns[i].y + ns[j].y !== p.y * 2) {
	    continue;
	  }
          pieces = pieces.filter(p1 => (p1.x != ns[i].x || p1.y != ns[i].y) && (p1.x != ns[j].x || p1.y != ns[j].y));
	}
      }
      return pieces;
    },
    movable: function(p) {
      return this.neighbors(p).filter(n => !this.is_piece(n));
    },
    pickup_piece: function(x, y) {
      let p = this.pieces.filter(p1 => p1.x == x && p1.y == y)[0];
      this.moving = p;
      this.blacks = [];
      this.remove_piece(p);
      for (let n of this.movable(p)) {
	this.markers.push({'x' : n.x, 'y' : n.y, 'r' : 10});
      }
    },
    drop_piece: function(x, y) {
      let m = {x : x, y : y, turn : +this.turn};
      if (this.turn == 0){
	this.pieces = this.capture_browns(m, this.pieces);
      }
      this.pieces = this.pieces.concat([m]);
      this.moving = null;
      this.markers = [];
      this.turn = 1 - this.turn;
      this.cur++;  
      this.save_state(); 
    },
    make_move: function(m) {
      this.pickup_piece(m.x1, m.y1);
      this.drop_piece(m.x2, m.y2);
    },   
    next_state: function(m) {
      let state = {};
      let p = this.pieces.filter(p1 => p1.x == m.x1 && p1.y == m.y1)[0];
      let pieces = this.pieces.filter(p1 => p1.x !== m.x1 || p1.y !== m.y1);
      let dst = {x : m.x2, y : m.y2, turn : +this.turn};
      if (this.turn == 0){
	pieces = this.capture_browns(dst, pieces);
      }
      return {pieces: pieces.concat([dst]), turn : 1 - this.turn};
    },  
    click_marker: function(p) {
      if (this.mode == 0) {
	this.pieces = this.pieces.concat([{x : p.x, y : p.y, turn : this.moving.turn}]);
        this.moving = null;
	this.markers = [];
      }
      else {
	this.drop_piece(p.x, p.y);
      }	
    },
    click_other: function() {
    },
    click_point: function(p) {
      if (this.is_piece(p)) {
	return;
      }
      if (this.mode == 0) {
	this.pieces.push({'x' : p.x, 'y' : p.y, turn : 1});
      }
      else if (this.moving != null) {
	this.pieces.push(this.moving);                         
	this.moving = null;
	this.markers = [];
      }
    },
    click_piece: function(p) {
      if (this.mode == 0) {
	this.remove_piece(p);
	if (p.turn == 0) {
	  this.moving = p;
	  for (let p1 of this.points) {
	    if (!this.is_piece(p1)) {
	      this.markers.push({'x' : p1.x, 'y' : p1.y, 'r' : 10});
	    }                           
	  }
	}
      }
      else if (this.turn == p.turn) {
        this.pickup_piece(p.x, p.y);
      }
    },
    next_moves: function() {
      let ms = [];
      for (let p of this.pieces.filter(p1 => p1.turn == this.turn)) {
	for (let n of this.movable(p)) {
          ms.push({x1 : p.x, y1 : p.y, x2 : n.x, y2 : n.y});
	}  
      }	
      return ms;
    },  
    save_state: function() {
      this.history = this.history.slice(0, this.cur);
      this.history.push({'pieces': this.pieces, 'turn': +this.turn}); 
    },       
    restore_state: function() {
      this.pieces = this.history[this.cur].pieces;
      this.turn = this.history[this.cur].turn;
      this.mode = 1;
    },
    can_fb: function () {
      return this.mode == 1 && this.moving == null;
    },   
    forward: function() {
      if (this.history.length > this.cur + 1) {
        this.cur++;
	this.restore_state();
      }
    },   
    backward: function() {
      if (this.cur > 0) {
        this.cur--;
        // console.log('cur=' + this.cur + ',history.length=' + this.history.length + ',this.history[this.cur]=' + JSON.stringify(this.history[this.cur]));
	this.restore_state();
      }
    },   
    rewind: function() {
      this.cur = 0;
      this.restore_state();
    },   
    last: function() {
      this.cur = this.history.length - 1;
      this.restore_state();
    },   
    best_move: async function() {
      if (this.mode == 0) {
        this.mode = 1;
      }	 
      if (this.moving != null) {
        this.pieces = this.pieces.concat([this.moving]);
      }	
      var best_next_val = (this.bval === 0 ? 0 :  this.bval - 1);
      for (let m of this.next_moves()) {
	let ns = this.next_state(m);
	// console.log('ns=' + JSON.stringify(ns));
        await this.board_value(ns);
	if (ns.bval == best_next_val) {
          this.save_state();
          this.make_move(m);
          return;
	}  
      }	
    },   
    reset_board: function() {
      this.history = [];
      this.points = [];
      this.lines = [];
      this.markers = [];
      this.pieces = [];
      this.message = "";
      this.mode = 0;
      this.cur = 0;
      if (this.init != null) {
        const istr = this.init.replace(/,/g, "");
        if (istr.length != [35, 35, 25][this.size]) {
          console.log('istr.length=' + JSON.stringify(istr.length));
        } else {
          console.log('length OK');
          m = istr.match(/X/g);
          if (m == null || m.length != 1) {
            console.log('black piece missmatch');
          } else {
            const width = [7, 7, 5][this.size];
            for (let y = 0; y < 5; y++) {
	      for (let x = 0; x < width; x++) {
	        if (this.has_point(x, y)) {
	          this.points.push({'x' : x, 'y' : y, 'r' : 2.5, 'color' : 'gray'});
	        }
                const c = istr[y * width + x];
                if (c == 'o') {
	          this.pieces.push({'x' : x, 'y' : y, 'turn' : 1});
	        } else if (c == 'X') {
	          this.pieces.push({'x' : x, 'y' : y, 'turn' : 0});
	        }
	        for (let n of this.neighbors({'x' : x, 'y' : y}, true)) {
	          this.lines.push({'x1' : x, 'y1' : y, 'x2' : n.x, 'y2' : n.y});
	        }
              }
            }
            this.mode = 1;
            this.save_state();
            return;
          }
        }
      }
      this.turn = 1;
      black = [2, 2];
      for (let y = 0; y < 5; y++) {
	for (let x = 0; x < 7; x++) {
	  if (this.has_point(x, y)) {
	    this.points.push({'x' : x, 'y' : y, 'r' : 2.5, 'color' : 'gray'});
	  }
	  if (has_brown[y][x]) {
	    this.pieces.push({'x' : x, 'y' : y, 'turn' : 1});
	  }
	  if (x == black[0] && y == black[1]) {
	    this.pieces.push({'x' : x, 'y' : y, 'turn' : 0});
	  }
	  for (let n of this.neighbors({'x' : x, 'y' : y}, true)) {
	    this.lines.push({'x1' : x, 'y1' : y, 'x2' : n.x, 'y2' : n.y});
	  }
	}
      }
      this.save_state();
    },
    play_mode: function() {
      if (this.mode != 1) {
        this.save_state(); 
      }	 
      else if (this.mode == 2) {
	this.restore_state();
      }
    },
    edit_mode: function() {
      if (this.mode == 2) {
	this.restore_state();
      } 	  
    },
    board_mode: function() {
      if (this.mode != 2) {
        this.save_state(); 
      } 	  
      this.pieces = []
    },
    change_board: function() {
      this.reset_board();
      console.log("size=" + this.size + " not supprted");
    }
  },
  created: function () {
    const param = location.search.substring(1);
    if (param) {
        const params = param.split('&');
        const paramArray = [];
        for (let p of params) {
            var cols = p.split('=');
            if (cols.length == 2) {
                paramArray[cols[0]] = cols[1];
            }
        }
        this.size = 0;
        if ("size" in paramArray) {
            i = ["33", "31", "25"].indexOf(paramArray["size"]);
            if (i >= 0) {
                this.size = i;
            }
        }
        this.turn = 1;
        if ("turn" in paramArray) {
            i = ["black", "brown"].indexOf(paramArray["turn"]);
            if (i >= 0) {
                this.turn = i;
            }
        }
        if ("init" in paramArray) {
            this.init = paramArray["init"];
        }
    }
    this.reset_board();
  }
});
