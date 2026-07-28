#!/usr/bin/env ruby
# frozen_string_literal: true

# A071091
# Independent enumeration by perfect matchings, without using the product
# formula for A071091.
#
# The region is the hexagon with alternating side lengths 2*n and 2*n+3,
# with the middle unit triangle removed from each of its three longer sides.
# Unit triangles are vertices of a planar bipartite graph, and lozenge tilings
# are exactly its perfect matchings.  This program constructs that graph,
# finds a Kasteleyn signing, and evaluates the absolute value of the resulting
# determinant by exact sparse elimination.
#
# Usage:
#   ruby 071091_02.rb       # n = 0..6
#   ruby 071091_02.rb 8     # n = 0..8

Triangle = Struct.new(:up, :i, :j, :x3, :y3, keyword_init: true)

LATTICE_DIRECTION_ORDER = {
  [-1, -1] => 0,
  [1, -2] => 1,
  [2, -1] => 2,
  [1, 1] => 3,
  [-1, 2] => 4,
  [-2, 1] => 5
}.freeze

def inside_convex_polygon?(x, y, polygon)
  polygon.each_with_index.all?{|(ax, ay), index|
    bx, by = polygon[(index + 1) % polygon.length]
    (bx - ax) * (y - ay) - (by - ay) * (x - ax) >= 0
  }
end

def border_punctured_hexagon(n)
  short_side = 2 * n
  long_side = 2 * n + 3

  # Coordinates use the two unit directions of the triangular lattice.
  # Multiplication by 3 makes all triangle-centroid coordinates integral.
  polygon = [
    [0, 0],
    [short_side, 0],
    [short_side, long_side],
    [0, short_side + long_side],
    [-long_side, short_side + long_side],
    [-long_side, long_side]
  ].map{|x, y| [3 * x, 3 * y]}

  # The three longer sides have odd length 2*n+3, hence a unique middle edge.
  # The inward-facing triangle incident with each such edge is down-pointing.
  missing = {
    [short_side - 1, n + 1] => true,
    [-n - 2, short_side + long_side - 1] => true,
    [-n - 2, n + 1] => true
  }

  triangles = []
  removed = 0
  (-long_side - 1).upto(short_side){|i|
    -1.upto(short_side + long_side){|j|
      ux = 3 * i + 1
      uy = 3 * j + 1
      if inside_convex_polygon?(ux, uy, polygon)
        triangles << Triangle.new(up: true, i: i, j: j, x3: ux, y3: uy)
      end

      dx = 3 * i + 2
      dy = 3 * j + 2
      next unless inside_convex_polygon?(dx, dy, polygon)

      if missing[[i, j]]
        removed += 1
      else
        triangles << Triangle.new(up: false, i: i, j: j, x3: dx, y3: dy)
      end
    }
  }

  raise "incorrect punctures: removed #{removed} triangles" unless removed == 3

  triangles
end

def build_graph(n)
  triangles = border_punctured_hexagon(n)
  up = triangles.select(&:up)
  down = triangles.reject(&:up)
  expected_per_color = 3 * (4 * n * n + 6 * n + 1)

  unless up.length == expected_per_color && down.length == expected_per_color
    raise "incorrect region: got #{up.length} up and #{down.length} down triangles"
  end

  id_by_triangle = {}
  triangles.each_with_index{|triangle, id|
    id_by_triangle[[triangle.up, triangle.i, triangle.j]] = id
  }
  adjacency = Array.new(triangles.length){[]}
  edge_ids = {}
  edge_count = 0

  up.each{|triangle|
    u = id_by_triangle.fetch([true, triangle.i, triangle.j])
    neighbors = [
      [false, triangle.i, triangle.j],
      [false, triangle.i, triangle.j - 1],
      [false, triangle.i - 1, triangle.j]
    ]

    neighbors.each{|key|
      v = id_by_triangle[key]
      next unless v

      edge_ids[[u, v].minmax] = edge_count
      edge_count += 1
      adjacency[u] << v
      adjacency[v] << u
    }
  }

  [triangles, up, down, adjacency, edge_ids, id_by_triangle]
end

def neighbor_direction_order(from, to)
  dx = to.x3 - from.x3
  dy = to.y3 - from.y3
  LATTICE_DIRECTION_ORDER.fetch([dx, dy])
end

def bounded_faces(triangles, adjacency, edge_ids)
  cyclic_neighbors = adjacency.each_with_index.map{|neighbors, vertex|
    neighbors.sort_by{|neighbor|
      neighbor_direction_order(triangles[vertex], triangles[neighbor])
    }
  }

  visited = {}
  faces = []

  cyclic_neighbors.each_with_index{|neighbors, start_u|
    neighbors.each{|start_v|
      next if visited[[start_u, start_v]]

      vertices = []
      face_edges = []
      u = start_u
      v = start_v

      loop{
        visited[[u, v]] = true
        vertices << u
        face_edges << edge_ids.fetch([u, v].minmax)

        around_v = cyclic_neighbors[v]
        incoming = around_v.index(u)
        raise "broken planar embedding" unless incoming

        w = around_v[(incoming - 1) % around_v.length]
        u = v
        v = w
        break if u == start_u && v == start_v
      }

      area_twice = vertices.each_with_index.sum{|vertex, index|
        following = vertices[(index + 1) % vertices.length]
        a = triangles[vertex]
        b = triangles[following]
        a.x3 * b.y3 - a.y3 * b.x3
      }

      faces << face_edges if area_twice.positive?
    }
  }

  faces
end

def bit_parity(value)
  parity = 0
  until value.zero?
    value &= value - 1
    parity ^= 1
  end
  parity
end

def kasteleyn_negative_edges(faces)
  # For a face of length 2m, the product of its edge signs must be
  # (-1)^(m+1).  Solve these parity equations over GF(2).
  pivots = {}

  faces.each{|face|
    raise "non-bipartite face" if face.length.odd?

    mask = face.reduce(0){|bits, edge| bits ^ (1 << edge)}
    rhs = (face.length / 2 + 1) & 1

    until mask.zero?
      pivot = mask.bit_length - 1
      previous = pivots[pivot]
      unless previous
        pivots[pivot] = [mask, rhs]
        break
      end

      mask ^= previous[0]
      rhs ^= previous[1]
    end

    raise "inconsistent Kasteleyn equations" if mask.zero? && rhs == 1
  }

  solution = 0
  pivots.keys.sort.each{|pivot|
    mask, rhs = pivots.fetch(pivot)
    known_part = mask ^ (1 << pivot)
    value = rhs ^ bit_parity(known_part & solution)
    solution |= 1 << pivot if value == 1
  }
  solution
end

def sparse_determinant_abs(input_rows, column_count)
  n = input_rows.length
  raise ArgumentError, "matrix must be square" unless column_count == n

  rows = {}
  column_rows = Array.new(column_count){{}}
  degree_buckets = []
  input_rows.each_with_index{|input_row, row_id|
    row = input_row.dup
    rows[row_id] = row
    row.each_key{|column| column_rows.fetch(column)[row_id] = true}
    (degree_buckets[row.length] ||= {})[row_id] = true
  }

  minimum_degree =
    degree_buckets.index{|bucket| bucket && !bucket.empty?} || 0
  determinant = 1

  until rows.empty?
    while !degree_buckets[minimum_degree] ||
          degree_buckets[minimum_degree].empty?
      minimum_degree += 1
      raise "broken degree index" if minimum_degree >= degree_buckets.length
    end

    pivot_row_id = degree_buckets[minimum_degree].each_key.first
    degree_buckets[minimum_degree].delete(pivot_row_id)
    pivot_row = rows.delete(pivot_row_id)
    return 0 if pivot_row.empty?

    pivot_column =
      pivot_row.keys.min_by{|column| column_rows[column].length}
    pivot = pivot_row.delete(pivot_column)
    determinant *= pivot

    pivot_row.each_key{|column| column_rows[column].delete(pivot_row_id)}
    column_rows[pivot_column].delete(pivot_row_id)
    affected_rows = column_rows[pivot_column].keys
    column_rows[pivot_column].clear

    affected_rows.each{|row_id|
      row = rows.fetch(row_id)
      old_degree = row.length
      entry = row.delete(pivot_column)
      factor = entry.quo(pivot)

      pivot_row.each{|column, value|
        old_value = row[column]
        updated = (old_value || 0) - factor * value
        if updated.zero?
          if old_value
            row.delete(column)
            column_rows[column].delete(row_id)
          end
        else
          row[column] = updated
          column_rows[column][row_id] = true unless old_value
        end
      }

      new_degree = row.length
      if new_degree != old_degree
        degree_buckets[old_degree].delete(row_id)
        (degree_buckets[new_degree] ||= {})[row_id] = true
        minimum_degree = new_degree if new_degree < minimum_degree
      end
    }
  end

  unless determinant.denominator == 1
    raise ArithmeticError, "nonintegral determinant"
  end

  determinant.numerator.abs
end

def a071091_by_matchings(n)
  unless n.is_a?(Integer) && n >= 0
    raise ArgumentError, "n must be a nonnegative integer"
  end

  triangles, up, down, adjacency, edge_ids, id_by_triangle = build_graph(n)
  faces = bounded_faces(triangles, adjacency, edge_ids)
  negative_edges = kasteleyn_negative_edges(faces)

  down_column = {}
  down.each_with_index{|triangle, column|
    down_column[id_by_triangle.fetch([false, triangle.i, triangle.j])] = column
  }

  matrix_rows = Array.new(up.length){{}}
  up.each_with_index{|triangle, row|
    u = id_by_triangle.fetch([true, triangle.i, triangle.j])
    adjacency[u].each{|v|
      edge = edge_ids.fetch([u, v].minmax)
      matrix_rows[row][down_column.fetch(v)] =
        negative_edges[edge] == 1 ? -1 : 1
    }
  }

  sparse_determinant_abs(matrix_rows, down.length)
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 6).to_i
  raise ArgumentError, "limit must be nonnegative" if limit.negative?

  0.upto(limit){|n|
    value = a071091_by_matchings(n)
    break if value.to_s.size > 1000
    puts "#{n} #{value}"
  }
end
