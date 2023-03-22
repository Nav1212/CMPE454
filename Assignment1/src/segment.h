// segment.h

class Segment {
   public:
    vec3 tail, head;

    Segment(vec3 t, vec3 h) {
        tail = t;
        head = h;
    }

    bool intersects(Segment const& s2) {
        // YOUR CODE HERE
        
        vec3 seg1Head = head;
        vec3 seg1Tail = tail;
        vec3 seg2Head = s2.head;
        vec3 seg2Tail = s2.tail;

        float denominator = ((seg1Tail.x - seg1Head.x) * (seg2Tail.y - seg2Head.y)) -
                             ((seg1Tail.y - seg1Head.y) * (seg2Tail.x - seg2Head.x));
        
        // lines are parallel if denominator of slope is 0
        if (denominator == 0) return false;

        // if not parallel, we check if the lines intersect in the space, and if that
        // intersection occurs withing the range of both lines
        float numerator1 = ((seg1Head.y - seg2Head.y) * (seg2Tail.x - seg2Head.x)) -
                            ((seg1Head.x - seg2Head.x) * (seg2Tail.y - seg2Head.y));
        float numerator2 = ((seg1Head.y - seg2Head.y) * (seg1Tail.x - seg1Head.x)) -
                            ((seg1Head.x - seg2Head.x) * (seg1Tail.y - seg1Head.y));

        float num1 = numerator1 / denominator;
        float num2 = numerator2 / denominator;

        return (num1 >= 0 && num1 <= 1) && (num2 >= 0 && num2 <= 1);
    }
};
